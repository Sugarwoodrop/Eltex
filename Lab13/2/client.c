#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "protocol.h"

#define SUCCESS 0
#define ERROR  -1
#define NAMES_W 22  
#define INPUT_H 3 
#define MAX_LINES 500

static int server_qid = ERROR;
static int my_qid     = ERROR;
static char my_name[MAX_NAME];
static volatile int running = 1;

static WINDOW *msg_win, *names_win, *input_win;
static pthread_mutex_t screen_lock = PTHREAD_MUTEX_INITIALIZER;

static char msg_lines[MAX_LINES][MAX_NAME + MAX_TEXT + 8];
static int  msg_count = 0;
static char names_buf[MAX_TEXT] = "";


static void draw_messages(void) {
    werase(msg_win);
    box(msg_win, 0, 0);
    mvwprintw(msg_win, 0, 2, " Сообщения ");
    int rows = getmaxy(msg_win) - 2;
    int start = msg_count > rows ? msg_count - rows : 0;
    int y = 1;
    for (int i = start; i < msg_count; i++)
        mvwprintw(msg_win, y++, 1, "%.*s", getmaxx(msg_win) - 2, msg_lines[i]);
    wrefresh(msg_win);
}

static void draw_names(void) {
    werase(names_win);
    box(names_win, 0, 0);
    mvwprintw(names_win, 0, 2, " Имена ");
    int y = 1;
    char tmp[MAX_TEXT];
    strncpy(tmp, names_buf, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    char *tok = strtok(tmp, "\n");
    while (tok && y < getmaxy(names_win) - 1) {
        mvwprintw(names_win, y++, 1, "%.*s", getmaxx(names_win) - 2, tok);
        tok = strtok(NULL, "\n");
    }
    wrefresh(names_win);
}

static void draw_input(const char *buf) {
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "> %.*s", getmaxx(input_win) - 4, buf);
    wrefresh(input_win);
}

static void push_line(const char *line) {
    if (msg_count < MAX_LINES) {
        strncpy(msg_lines[msg_count], line, MAX_NAME + MAX_TEXT + 7);
        msg_lines[msg_count][MAX_NAME + MAX_TEXT + 7] = '\0';
        msg_count++;
    } else {
        memmove(msg_lines[0], msg_lines[1],
                (MAX_LINES - 1) * sizeof(msg_lines[0]));
        strncpy(msg_lines[MAX_LINES - 1], line, MAX_NAME + MAX_TEXT + 7);
        msg_lines[MAX_LINES - 1][MAX_NAME + MAX_TEXT + 7] = '\0';
    }
}

static void *receiver(void *arg) {
    (void)arg;
    struct notif n;
    while (running) {
        if (msgrcv(my_qid, &n, sizeof(struct notif) - sizeof(long), 1, 0) == ERROR) {
            if (errno == EINTR) continue;
            break;
        }
        char line[MAX_NAME + MAX_TEXT + 8];
        pthread_mutex_lock(&screen_lock);
        switch (n.kind) {
            case N_MSG:
                snprintf(line, sizeof(line), "%s: %s", n.name, n.text);
                push_line(line);
                draw_messages();
                break;
            case N_SYS:
                snprintf(line, sizeof(line), "*** %s %s ***", n.name, n.text);
                push_line(line);
                draw_messages();
                break;
            case N_NAMES:
                strncpy(names_buf, n.text, sizeof(names_buf) - 1);
                names_buf[sizeof(names_buf) - 1] = '\0';
                draw_names();
                break;
        }
        wmove(input_win, 1, 3);
        wrefresh(input_win);
        pthread_mutex_unlock(&screen_lock);
    }
    return NULL;
}

static void send_req(int cmd, const char *text) {
    struct req r;
    r.mtype     = 1;
    r.cmd       = cmd;
    r.reply_qid = my_qid;
    strncpy(r.name, my_name, MAX_NAME - 1);
    r.name[MAX_NAME - 1] = '\0';
    strncpy(r.text, text ? text : "", MAX_TEXT - 1);
    r.text[MAX_TEXT - 1] = '\0';
    msgsnd(server_qid, &r, sizeof(struct req) - sizeof(long), 0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <name>\n", argv[0]);
        return ERROR;
    }
    strncpy(my_name, argv[1], MAX_NAME - 1);
    my_name[MAX_NAME - 1] = '\0';

    /* подключаемся к очереди сервера */
    key_t key = ftok(KEY_PATH, KEY_ID);
    server_qid = msgget(key, 0);
    if (server_qid == ERROR) {
        fprintf(stderr, "Сервер не запущен (msgget: %s)\n", strerror(errno));
        return ERROR;
    }

    my_qid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (my_qid == ERROR) {
        perror("msgget private");
        return ERROR;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    refresh();

    int h = LINES, w = COLS;
    int top_h = h - INPUT_H;
    int msg_w = w - NAMES_W;

    msg_win   = newwin(top_h, msg_w, 0, 0);
    names_win = newwin(top_h, NAMES_W, 0, msg_w);
    input_win = newwin(INPUT_H, w, top_h, 0);
    keypad(input_win, TRUE);
    wtimeout(input_win, 200);

    pthread_mutex_lock(&screen_lock);
    draw_messages();
    draw_names();
    draw_input("");
    pthread_mutex_unlock(&screen_lock);

    send_req(CMD_JOIN, "");

    pthread_t tid;
    pthread_create(&tid, NULL, receiver, NULL);

    char input[MAX_TEXT];
    int len = 0;
    input[0] = '\0';

    while (running) {
        pthread_mutex_lock(&screen_lock);
        int ch = wgetch(input_win);
        pthread_mutex_unlock(&screen_lock);

        if (ch == ERR) continue;

        if (ch == '\n') {
            if (len > 0) {
                if (strcmp(input, "/quit") == 0) {
                    running = 0;
                    break;
                }
                send_req(CMD_MSG, input);
                len = 0;
                input[0] = '\0';
            }
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (len > 0) input[--len] = '\0';
        } else if (isprint(ch) && len < MAX_TEXT - 1) {
            input[len++] = (char)ch;
            input[len] = '\0';
        }

        pthread_mutex_lock(&screen_lock);
        draw_input(input);
        pthread_mutex_unlock(&screen_lock);
    }

    send_req(CMD_LEAVE, "");
    msgctl(my_qid, IPC_RMID, NULL);
    pthread_join(tid, NULL);

    endwin();
    return SUCCESS;
}
