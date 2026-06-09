/* 
  Вариант 1: на каждого клиента — отдельный поток обслуживания,
  который завершается по окончании обслуживания.
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
#define ERROR -1

static void send_time(int fd) {
    time_t now = time(NULL);
    char *t = ctime(&now);
    write(fd, t, strlen(t));
}

static void *worker(void *arg) {
    int fd = *(int *)arg;
    free(arg);
    pthread_detach(pthread_self());

    char tmp[64];
    recv(fd, tmp, sizeof(tmp), 0); 
    send_time(fd);

    close(fd);
    return NULL;
}

int main() {
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
    printf("Time server (thread-per-client) on port %d\n", PORT);

    while (1) {
        int c = accept(s, NULL, NULL);
        if (c == ERROR) { perror("accept"); continue; }
        int *arg = malloc(sizeof(int));
        *arg = c;
        pthread_t tid;
        if (pthread_create(&tid, NULL, worker, arg) != 0) {
            perror("pthread_create");
            close(c);
            free(arg);
        }
    }

    close(s);
    return 0;
}
