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

    int qid = msgget(key, 0);
    if (qid == ERROR) {
        perror("msgget (запустите сначала сервер)");
        exit(ERROR);
    }

    struct msgbuf msg;
    if (msgrcv(qid, &msg, MAX_TEXT, TYPE_S2C, 0) == ERROR) {
        perror("msgrcv");
        exit(ERROR);
    }
    printf("Client received: %s\n", msg.mtext);

    msg.mtype = TYPE_C2S;
    strcpy(msg.mtext, "Hello!");
    if (msgsnd(qid, &msg, strlen(msg.mtext) + 1, 0) == ERROR) {
        perror("msgsnd");
        exit(ERROR);
    }

    return SUCCESS;
}
