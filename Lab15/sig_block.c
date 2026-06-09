#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define SUCCESS 0
#define ERROR  -1

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == ERROR) {
        perror("sigprocmask");
        exit(ERROR);
    }

    printf("PID = %d. SIGINT заблокирован.\n", getpid());
    printf("Ctrl+C / 'kill -INT %d' игнорируются.\n", getpid());
    printf("Остановить можно через SIGTERM ('kill %d') или SIGKILL.\n", getpid());
    fflush(stdout);

    while (1) {
        sleep(1);
    }

    return SUCCESS;
}
