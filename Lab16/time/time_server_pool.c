/*
  Вариант 2: при запуске создаётся пул потоков обслуживания.
  Слушающий поток принимает соединение и передаёт его свободному
  воркеру; воркер по завершении сообщает, что освободился.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 7000
#define POOL 4
#define ERROR -1

typedef struct {
    pthread_t       tid;
    int             fd;
    int             busy;
    pthread_mutex_t m;
    pthread_cond_t  cv;
} worker_t;

static worker_t pool[POOL];
static pthread_mutex_t pool_m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  freed   = PTHREAD_COND_INITIALIZER;

static void send_time(int fd) {
    time_t now = time(NULL);
    char *t = ctime(&now);
    write(fd, t, strlen(t));
}

static void *worker_fn(void *arg) {
    worker_t *w = arg;
    while (1) {
        pthread_mutex_lock(&w->m);
        while (w->fd < 0)
            pthread_cond_wait(&w->cv, &w->m);
        int fd = w->fd;
        pthread_mutex_unlock(&w->m);

        char tmp[64];
        recv(fd, tmp, sizeof(tmp), 0);
        send_time(fd);
        close(fd);

        pthread_mutex_lock(&pool_m);
        w->busy = 0;
        pthread_mutex_lock(&w->m);
        w->fd = -1;
        pthread_mutex_unlock(&w->m);
        pthread_cond_signal(&freed);
        pthread_mutex_unlock(&pool_m);
    }
    return NULL;
}

int main() {
    for (int i = 0; i < POOL; i++) {
        pool[i].fd = -1;
        pool[i].busy = 0;
        pthread_mutex_init(&pool[i].m, NULL);
        pthread_cond_init(&pool[i].cv, NULL);
        pthread_create(&pool[i].tid, NULL, worker_fn, &pool[i]);
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == ERROR) { perror("bind"); exit(ERROR); }
    if (listen(s, 16) == ERROR) { perror("listen"); exit(ERROR); }
    printf("Time server (thread pool, %d workers) on port %d\n", POOL, PORT);

    while (1) {
        int c = accept(s, NULL, NULL);
        if (c == ERROR) { perror("accept"); continue; }

        pthread_mutex_lock(&pool_m);
        int idx = -1;
        while (idx < 0) {
            for (int i = 0; i < POOL; i++)
                if (!pool[i].busy) { idx = i; break; }
            if (idx < 0)
                pthread_cond_wait(&freed, &pool_m);
        }
        pool[idx].busy = 1;
        pthread_mutex_unlock(&pool_m);

        pthread_mutex_lock(&pool[idx].m);
        pool[idx].fd = c;
        pthread_cond_signal(&pool[idx].cv);
        pthread_mutex_unlock(&pool[idx].m);
    }

    close(s);
    return 0;
}
