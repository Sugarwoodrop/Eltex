#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#define SUCCESS 0
#define ERROR  -1
#define KEY_PATH "/tmp"
#define KEY_ID   'S'
#define MAX_MSG  256

#define SEM_SRV 0
#define SEM_CLI 1

static void sem_op(int semid, int num, int op) {
    struct sembuf sb = { (unsigned short)num, (short)op, 0 };
    if (semop(semid, &sb, 1) == ERROR) {
        perror("semop");
        exit(ERROR);
    }
}

int main() {
    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == ERROR) {
        perror("ftok");
        exit(ERROR);
    }

    int shmid = shmget(key, MAX_MSG, 0);
    int semid = semget(key, 2, 0);
    if (shmid == ERROR || semid == ERROR) {
        perror("shmget/semget (запустите сначала сервер)");
        exit(ERROR);
    }

    char *buf = shmat(shmid, NULL, 0);
    if (buf == (char *)ERROR) {
        perror("shmat");
        exit(ERROR);
    }

    sem_op(semid, SEM_SRV, -1);
    printf("Client received: %s\n", buf);

    strcpy(buf, "Hello!");
    sem_op(semid, SEM_CLI, +1);

    shmdt(buf);

    return SUCCESS;
}
