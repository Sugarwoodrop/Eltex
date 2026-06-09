#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "protocol.h"

#define SUCCESS 0
#define ERROR  -1

typedef struct {
    int  qid;
    char name[MAX_NAME];
    int  active;
} client_t;

static client_t clients[MAX_CLIENTS];
static int       nclients = 0;

static char      history[MAX_HISTORY][MAX_NAME + MAX_TEXT];
static int       hist_count = 0;

static int server_qid = ERROR;

static void cleanup(int sig) {
    (void)sig;
    if (server_qid != ERROR)
        msgctl(server_qid, IPC_RMID, NULL);
    exit(SUCCESS);
}

static void notify(client_t *c, int kind, const char *name, const char *text) {
    if (!c->active) return;
    struct notif n;
    n.mtype = 1;
    n.kind  = kind;
    strncpy(n.name, name ? name : "", MAX_NAME - 1);
    n.name[MAX_NAME - 1] = '\0';
    strncpy(n.text, text ? text : "", MAX_TEXT - 1);
    n.text[MAX_TEXT - 1] = '\0';
    if (msgsnd(c->qid, &n, sizeof(struct notif) - sizeof(long), IPC_NOWAIT) == ERROR)
        c->active = 0;
}

static void broadcast(int kind, const char *name, const char *text) {
    for (int i = 0; i < nclients; i++)
        notify(&clients[i], kind, name, text);
}

static void broadcast_names(void) {
    char buf[MAX_TEXT];
    buf[0] = '\0';
    for (int i = 0; i < nclients; i++) {
        if (!clients[i].active) continue;
        strncat(buf, clients[i].name, sizeof(buf) - strlen(buf) - 2);
        strncat(buf, "\n", sizeof(buf) - strlen(buf) - 1);
    }
    broadcast(N_NAMES, "", buf);
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == ERROR) {
        perror("ftok");
        exit(ERROR);
    }

    server_qid = msgget(key, IPC_CREAT | 0666);
    if (server_qid == ERROR) {
        perror("msgget");
        exit(ERROR);
    }

    printf("Chat server started. Waiting for clients... (Ctrl+C to stop)\n");

    struct req r;
    while (1) {
        if (msgrcv(server_qid, &r, sizeof(struct req) - sizeof(long), 1, 0) == ERROR) {
            if (errno == EINTR) continue;
            perror("msgrcv");
            break;
        }

        if (r.cmd == CMD_JOIN) {
            if (nclients >= MAX_CLIENTS) continue;
            client_t *c = &clients[nclients++];
            c->qid = r.reply_qid;
            strncpy(c->name, r.name, MAX_NAME - 1);
            c->name[MAX_NAME - 1] = '\0';
            c->active = 1;

            printf("[+] %s joined (qid=%d)\n", c->name, c->qid);

            for (int i = 0; i < hist_count; i++) {
                char *line = history[i];
                char *sep = strchr(line, '\x1f'); 
                if (sep) {
                    *sep = '\0';
                    notify(c, N_MSG, line, sep + 1);
                    *sep = '\x1f';
                }
            }
            broadcast_names();
            for (int i = 0; i < nclients - 1; i++)
                notify(&clients[i], N_SYS, c->name, "joined the chat");

        } else if (r.cmd == CMD_MSG) {
            if (hist_count < MAX_HISTORY) {
                snprintf(history[hist_count], MAX_NAME + MAX_TEXT,
                         "%s\x1f%s", r.name, r.text);
                hist_count++;
            }
            printf("[msg] %s: %s\n", r.name, r.text);
            broadcast(N_MSG, r.name, r.text);

        } else if (r.cmd == CMD_LEAVE) {
            for (int i = 0; i < nclients; i++) {
                if (clients[i].active && strcmp(clients[i].name, r.name) == 0
                    && clients[i].qid == r.reply_qid) {
                    clients[i].active = 0;
                    printf("[-] %s left\n", clients[i].name);
                    break;
                }
            }
            broadcast_names();
            broadcast(N_SYS, r.name, "left the chat");
        }
    }

    cleanup(0);
    return SUCCESS;
}
