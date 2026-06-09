#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define SUCCESS 0
#define ERROR  -1

static void handler(int sig) {
    (void)sig;
    const char *msg = "Получен сигнал SIGUSR1\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) == ERROR) {
        perror("sigaction");
        exit(ERROR);
    }

    printf("Receiver PID = %d. Ожидаю SIGUSR1...\n", getpid());
    fflush(stdout);

    while (1) {
        pause();
    }

    return SUCCESS;
}
