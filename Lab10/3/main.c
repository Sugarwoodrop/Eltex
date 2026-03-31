#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#define SUCCESS 0
#define ERROR -1
#define MAX_LENGTH 1024
#define MAX_ARGS 64

int main(){
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

        char *args[MAX_ARGS];
        int argc = 0;

        char *token = strtok(line, " \t");
        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }
        args[argc] = NULL;

        if (argc == 0) continue;

        pid_t pid = fork();

        if (pid == ERROR) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(args[0], args);
            perror(args[0]);
            exit(ERROR);
        }

        int stat;
        wait(&stat);
        if (WIFEXITED(stat)) {
            printf("[exited: %d]\n", WEXITSTATUS(stat));
        }
    }
    return SUCCESS;
}
