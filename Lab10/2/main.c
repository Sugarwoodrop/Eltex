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

void first_proc(){
    printf("First ");
    print_pid_ppid();
    for(int i = 0; i < 2; i++){
        pid_t child_pid = fork();
        if (child_pid == ERROR) {
            perror("fork failed");
            return;
        }
        if(child_pid == 0){
            if(i == 0){
                printf("Three ");
                print_pid_ppid();
                sleep(1);
                _exit(32);
            }
            if(i == 1){
                printf("Four ");
                print_pid_ppid();
                sleep(1);
                _exit(42);
            }
        }
    }
    for(int i = 0; i<2; i++){    
        int stat;
        pid_t finished = wait(&stat);
        if (WIFEXITED(stat)) {
            printf("1.Child (pid %d) exited with code: %d\n", finished, WEXITSTATUS(stat));
        } else if (WIFSIGNALED(stat)) {
            printf("1.Child killed by signal: %d\n", WTERMSIG(stat));
        }
    }
}

void second_proc(){
    printf("Second ");
    print_pid_ppid();
    pid_t child_pid = fork();
    if (child_pid == ERROR) {
        perror("fork failed");
        return;
    }
    if(child_pid == 0){
        printf("Five ");
        print_pid_ppid();
        sleep(1);
        _exit(52);
    }
    int stat;
    pid_t finished = wait(&stat);
    if (WIFEXITED(stat)) {
        printf("2.Child (pid %d) exited with code: %d\n", finished, WEXITSTATUS(stat));
    } else if (WIFSIGNALED(stat)) {
        printf("2.Child killed by signal: %d\n", WTERMSIG(stat));
    }
}

int main(){
    printf("Parent ");
    print_pid_ppid();
    for(int i = 0; i < 2; i++){
        pid_t child_pid = fork();
        if (child_pid == ERROR) {
            perror("fork failed");
            return ERROR;
        }
        if(child_pid == 0){
            if(i == 0){
                first_proc();
                return 12;
            }
            if(i == 1){
                second_proc();
                return 22;
            }
        }
    }
    for(int i = 0; i<2; i++){    
        int stat;
        pid_t finished = wait(&stat);
        if (WIFEXITED(stat)) {
            printf("Child (pid %d) exited with code: %d\n",
                   finished, WEXITSTATUS(stat));
        } else if (WIFSIGNALED(stat)) {
            printf("Child killed by signal: %d\n", WTERMSIG(stat));
        }
    }
    return SUCCESS;
}
