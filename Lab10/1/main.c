#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SUCCESS 0
#define ERROR -1

void print_pid_ppid(){
    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("pid %d, ppid %d\n", pid, ppid);
}

int main(){
    printf("Parent ");
    print_pid_ppid();
    pid_t child_pid = fork();
       if (child_pid == ERROR) {
        perror("fork failed");
        return ERROR;
    }
    if(child_pid == 0){
        printf("Child ");
        print_pid_ppid();
        sleep(2);
        return 46;
    }
    int stat;
    pid_t finished = wait(&stat);
    if (WIFEXITED(stat)) {
        printf("Child (pid %d) exited with code: %d\n", finished, WEXITSTATUS(stat));
    } else if (WIFSIGNALED(stat)) {
        printf("Child killed by signal: %d\n", WTERMSIG(stat));
    }
    return SUCCESS;
}
