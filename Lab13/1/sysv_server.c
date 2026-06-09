#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define SUCCESS 0
#define ERROR  -1
#define KEY_PATH "/tmp"
#define KEY_ID   'M'
#define MAX_TEXT 256

#define TYPE_S2C 1   /* server -> client */
#define TYPE_C2S 2   /* client -> server */

struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT];
};

int main() {
    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == ERROR) {
        perror("ftok");
        exit(ERROR);
    }

    int qid = msgget(key, IPC_CREAT | 0666);
    if (qid == ERROR) {
        perror("msgget");
        exit(ERROR);
    }

    struct msgbuf msg;
    msg.mtype = TYPE_S2C;
    strcpy(msg.mtext, "Hi!");
    if (msgsnd(qid, &msg, strlen(msg.mtext) + 1, 0) == ERROR) {
        perror("msgsnd");
        exit(ERROR);
    }

    if (msgrcv(qid, &msg, MAX_TEXT, TYPE_C2S, 0) == ERROR) {
        perror("msgrcv");
        exit(ERROR);
    }
    printf("Server received: %s\n", msg.mtext);

    if (msgctl(qid, IPC_RMID, NULL) == ERROR) {
        perror("msgctl");
        exit(ERROR);
    }

    return SUCCESS;
}
