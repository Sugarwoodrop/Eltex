#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define SUCCESS 0
#define ERROR  -1

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return ERROR;
    }

    pid_t pid = (pid_t)atoi(argv[1]);

    if (kill(pid, SIGUSR1) == ERROR) {
        perror("kill");
        return ERROR;
    }

    printf("SIGUSR1 отправлен процессу %d\n", pid);
    return SUCCESS;
}
