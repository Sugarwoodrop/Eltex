#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>

#define SUCCESS 0
#define ERROR  -1
#define QUEUE_S2C "/q_s2c"   /* server -> client */
#define QUEUE_C2S "/q_c2s"   /* client -> server */
#define MAX_MSG_SIZE 256

int main() {
    mqd_t s2c = mq_open(QUEUE_S2C, O_RDONLY);
    mqd_t c2s = mq_open(QUEUE_C2S, O_WRONLY);
    if (s2c == (mqd_t)ERROR || c2s == (mqd_t)ERROR) {
        perror("mq_open (запустите сначала сервер)");
        exit(ERROR);
    }

    char buf[MAX_MSG_SIZE];
    ssize_t n = mq_receive(s2c, buf, sizeof(buf), NULL);
    if (n == ERROR) {
        perror("mq_receive");
        exit(ERROR);
    }
    printf("Client received: %s\n", buf);

    const char *hello = "Hello!";
    if (mq_send(c2s, hello, strlen(hello) + 1, 0) == ERROR) {
        perror("mq_send");
        exit(ERROR);
    }

    mq_close(s2c);
    mq_close(c2s);

    return SUCCESS;
}
