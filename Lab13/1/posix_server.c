#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SUCCESS 0
#define ERROR  -1
#define QUEUE_S2C "/q_s2c"   /* server -> client */
#define QUEUE_C2S "/q_c2s"   /* client -> server */
#define MAX_MSG_SIZE 256

int main() {
    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t s2c = mq_open(QUEUE_S2C, O_WRONLY | O_CREAT, 0666, &attr);
    mqd_t c2s = mq_open(QUEUE_C2S, O_RDONLY | O_CREAT, 0666, &attr);
    if (s2c == (mqd_t)ERROR || c2s == (mqd_t)ERROR) {
        perror("mq_open");
        exit(ERROR);
    }

    const char *hi = "Hi!";
    if (mq_send(s2c, hi, strlen(hi) + 1, 0) == ERROR) {
        perror("mq_send");
        exit(ERROR);
    }

    char buf[MAX_MSG_SIZE];
    ssize_t n = mq_receive(c2s, buf, sizeof(buf), NULL);
    if (n == ERROR) {
        perror("mq_receive");
        exit(ERROR);
    }
    printf("Server received: %s\n", buf);

    mq_close(s2c);
    mq_close(c2s);
    mq_unlink(QUEUE_S2C);
    mq_unlink(QUEUE_C2S);

    return SUCCESS;
}
