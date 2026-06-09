#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define ERROR  -1
#define MAX_LENGTH 1024
#define MAX_ARGS   64
#define MAX_CMDS   64
#define READ  0
#define WRITE 1

static int parse_args(char *cmd, char *args[]) {
    int argc = 0;
    char *token = strtok(cmd, " \t");
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }
    args[argc] = NULL;
    return argc;
}

int main() {
    char line[MAX_LENGTH];

    while (1) {
        printf("mySh>>");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "exit") == 0) {
            break;
        }

        char *cmds[MAX_CMDS];
        int ncmds = 0;
        char *p = strtok(line, "|");
        while (p != NULL && ncmds < MAX_CMDS) {
            cmds[ncmds++] = p;
            p = strtok(NULL, "|");
        }
        if (ncmds == 0) continue;

        int in_fd = STDIN_FILENO; 
        int fd[2];
        pid_t pids[MAX_CMDS];
        int nforked = 0;

        for (int i = 0; i < ncmds; i++) {
            char *args[MAX_ARGS];
            if (parse_args(cmds[i], args) == 0) {
                fprintf(stderr, "syntax error near '|'\n");
                break;
            }

            if (i < ncmds - 1) {
                if (pipe(fd) == ERROR) {
                    perror("pipe");
                    break;
                }
            }

            pid_t pid = fork();
            if (pid == ERROR) {
                perror("fork");
                break;
            }

            if (pid == 0) {
                if (in_fd != STDIN_FILENO) {
                    dup2(in_fd, STDIN_FILENO);
                    close(in_fd);
                }
                if (i < ncmds - 1) {
                    close(fd[READ]);
                    dup2(fd[WRITE], STDOUT_FILENO);
                    close(fd[WRITE]);
                }
                execvp(args[0], args);
                perror(args[0]);
                exit(ERROR);
            }

            pids[nforked++] = pid;

            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            if (i < ncmds - 1) {
                close(fd[WRITE]);
                in_fd = fd[READ];
            }
        }

        for (int i = 0; i < nforked; i++) {
            int stat;
            waitpid(pids[i], &stat, 0);
            if (i == nforked - 1 && WIFEXITED(stat)) {
                printf("[exited: %d]\n", WEXITSTATUS(stat));
            }
        }
    }

    return SUCCESS;
}
