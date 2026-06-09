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

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

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

    int shmid = shmget(key, MAX_MSG, IPC_CREAT | 0666);
    if (shmid == ERROR) {
        perror("shmget");
        exit(ERROR);
    }

    int semid = semget(key, 2, IPC_CREAT | 0666);
    if (semid == ERROR) {
        perror("semget");
        exit(ERROR);
    }

    union semun arg;
    arg.val = 0;
    semctl(semid, SEM_SRV, SETVAL, arg);
    semctl(semid, SEM_CLI, SETVAL, arg);

    char *buf = shmat(shmid, NULL, 0);
    if (buf == (char *)ERROR) {
        perror("shmat");
        exit(ERROR);
    }

    strcpy(buf, "Hi!");
    sem_op(semid, SEM_SRV, +1);

    sem_op(semid, SEM_CLI, -1);
    printf("Server received: %s\n", buf);

    shmdt(buf);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return SUCCESS;
}
