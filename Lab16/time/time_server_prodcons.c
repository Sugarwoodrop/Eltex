/* 
  Вариант 3: producer/consumer. Слушающий поток (производитель)
  кладёт принятые соединения в очередь. Пул воркеров (потребители)
  разбирают заявки из очереди и отвечают клиенту. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT  7000
#define POOL  4
#define QCAP  64
#define ERROR -1

static int queue[QCAP];
static int q_head = 0, q_tail = 0, q_size = 0;
static pthread_mutex_t q_m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  q_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  q_not_full  = PTHREAD_COND_INITIALIZER;

static void q_push(int fd) {
    pthread_mutex_lock(&q_m);
    while (q_size == QCAP)
        pthread_cond_wait(&q_not_full, &q_m);
    queue[q_tail] = fd;
    q_tail = (q_tail + 1) % QCAP;
    q_size++;
    pthread_cond_signal(&q_not_empty);
    pthread_mutex_unlock(&q_m);
}

static int q_pop(void) {
    pthread_mutex_lock(&q_m);
    while (q_size == 0)
        pthread_cond_wait(&q_not_empty, &q_m);
    int fd = queue[q_head];
    q_head = (q_head + 1) % QCAP;
    q_size--;
    pthread_cond_signal(&q_not_full);
    pthread_mutex_unlock(&q_m);
    return fd;
}

static void send_time(int fd) {
    time_t now = time(NULL);
    char *t = ctime(&now);
    write(fd, t, strlen(t));
}

static void *consumer(void *arg) {
    (void)arg;
    while (1) {
        int fd = q_pop();
        char tmp[64];
        recv(fd, tmp, sizeof(tmp), 0);
        send_time(fd);
        close(fd);
    }
    return NULL;
}

int main() {
    pthread_t tids[POOL];
    for (int i = 0; i < POOL; i++)
        pthread_create(&tids[i], NULL, consumer, NULL);

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
    printf("Time server (producer/consumer, %d workers) on port %d\n", POOL, PORT);

    while (1) {
        int c = accept(s, NULL, NULL);
        if (c == ERROR) { perror("accept"); continue; }
        q_push(c);
    }

    close(s);
    return 0;
}
