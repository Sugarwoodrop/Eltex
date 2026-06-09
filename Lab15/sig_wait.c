#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define SUCCESS 0
#define ERROR  -1

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == ERROR) {
        perror("sigprocmask");
        exit(ERROR);
    }

    printf("PID = %d. Event-loop на sigwait(), жду SIGUSR1...\n", getpid());
    fflush(stdout);

    while (1) {
        int sig;
        if (sigwait(&set, &sig) != SUCCESS) {
            perror("sigwait");
            exit(ERROR);
        }
        if (sig == SIGUSR1) {
            printf("Получен сигнал SIGUSR1, итерация цикла\n");
            fflush(stdout);
        }
    }

    return SUCCESS;
}
