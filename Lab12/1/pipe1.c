#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define ERROR  -1
#define READ    0
#define WRITE   1
#define MAX_LENGTH 1024

int main() {
    int fd[2];
    if (pipe(fd) == ERROR) {
        perror("pipe");
        exit(ERROR);
    }

    pid_t pid = fork();
    if (pid == ERROR) {
        perror("fork");
        exit(ERROR);
    }

    if (pid > 0) {
        close(fd[READ]);
        const char *msg = "Hi!";
        if (write(fd[WRITE], msg, strlen(msg)) == ERROR) {
            perror("write");
            exit(ERROR);
        }
        close(fd[WRITE]);
        wait(NULL);
    } else {
        close(fd[WRITE]);
        char buf[MAX_LENGTH];
        ssize_t n = read(fd[READ], buf, sizeof(buf) - 1);
        if (n == ERROR) {
            perror("read");
            exit(ERROR);
        }
        buf[n] = '\0';
        printf("Child received: %s\n", buf);
        close(fd[READ]);
    }

    return SUCCESS;
}
