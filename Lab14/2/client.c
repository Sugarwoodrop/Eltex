#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/mman.h>

#include "shm_chat.h"

#define SUCCESS 0
#define ERROR  -1
#define NAMES_W 22
#define INPUT_H 3

static shm_chat *chat = NULL;
static char my_name[MAX_NAME];
static volatile int running = 1;

static WINDOW *msg_win, *names_win, *input_win;
static pthread_mutex_t screen_lock = PTHREAD_MUTEX_INITIALIZER;

static void draw_input(const char *buf) {
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "> %.*s", getmaxx(input_win) - 4, buf);
    wrefresh(input_win);
}

static void redraw(chat_msg *snap, int from, int to, char users[][MAX_NAME], int ucount) {
    werase(msg_win);
    box(msg_win, 0, 0);
    mvwprintw(msg_win, 0, 2, " Сообщения ");
    int rows = getmaxy(msg_win) - 2;
    int total = to - from;
    int start = total > rows ? total - rows : 0;
    int y = 1;
    for (int i = start; i < total; i++) {
        chat_msg *m = &snap[i];
        char line[MAX_NAME + MAX_TEXT + 8];
        if (m->is_sys)
            snprintf(line, sizeof(line), "*** %s %s ***", m->name, m->text);
        else
            snprintf(line, sizeof(line), "%s: %s", m->name, m->text);
        mvwprintw(msg_win, y++, 1, "%.*s", getmaxx(msg_win) - 2, line);
    }
    wrefresh(msg_win);

    /* имена */
    werase(names_win);
    box(names_win, 0, 0);
    mvwprintw(names_win, 0, 2, " Имена ");
    for (int i = 0; i < ucount && i < getmaxy(names_win) - 2; i++)
        mvwprintw(names_win, i + 1, 1, "%.*s", getmaxx(names_win) - 2, users[i]);
    wrefresh(names_win);

    wmove(input_win, 1, 3);
    wrefresh(input_win);
}

static void post_msg(const char *text, int is_sys) {
    pthread_mutex_lock(&chat->lock);
    chat_msg *m = &chat->msgs[chat->total % MAX_MSGS];
    strncpy(m->name, my_name, MAX_NAME - 1);
    m->name[MAX_NAME - 1] = '\0';
    strncpy(m->text, text, MAX_TEXT - 1);
    m->text[MAX_TEXT - 1] = '\0';
    m->is_sys = is_sys;
    chat->total++;
    pthread_cond_broadcast(&chat->cond);
    pthread_mutex_unlock(&chat->lock);
}

static void add_user(void) {
    pthread_mutex_lock(&chat->lock);
    if (chat->user_count < MAX_USERS) {
        strncpy(chat->users[chat->user_count], my_name, MAX_NAME - 1);
        chat->users[chat->user_count][MAX_NAME - 1] = '\0';
        chat->user_count++;
    }
    pthread_mutex_unlock(&chat->lock);
    post_msg("joined the chat", 1);
}

static void remove_user(void) {
    pthread_mutex_lock(&chat->lock);
    for (int i = 0; i < chat->user_count; i++) {
        if (strcmp(chat->users[i], my_name) == 0) {
            for (int j = i; j < chat->user_count - 1; j++)
                strcpy(chat->users[j], chat->users[j + 1]);
            chat->user_count--;
            break;
        }
    }
    pthread_mutex_unlock(&chat->lock);
    post_msg("left the chat", 1);
}

static void *receiver(void *arg) {
    (void)arg;
    long seen = 0;
    chat_msg snap[MAX_MSGS];
    char users[MAX_USERS][MAX_NAME];
    int ucount;

    while (running) {
        pthread_mutex_lock(&chat->lock);
        while (chat->total == seen && running)
            pthread_cond_wait(&chat->cond, &chat->lock);
        if (!running) { pthread_mutex_unlock(&chat->lock); break; }

        long start = chat->total > MAX_MSGS ? chat->total - MAX_MSGS : 0;
        int n = 0;
        for (long i = start; i < chat->total; i++)
            snap[n++] = chat->msgs[i % MAX_MSGS];
        seen = chat->total;

        ucount = chat->user_count;
        memcpy(users, chat->users, sizeof(users));
        pthread_mutex_unlock(&chat->lock);

        pthread_mutex_lock(&screen_lock);
        redraw(snap, 0, n, users, ucount);
        pthread_mutex_unlock(&screen_lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <name>\n", argv[0]);
        return ERROR;
    }
    strncpy(my_name, argv[1], MAX_NAME - 1);
    my_name[MAX_NAME - 1] = '\0';

    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == ERROR) {
        fprintf(stderr, "Сервер не запущен (shm_open)\n");
        return ERROR;
    }
    chat = mmap(NULL, sizeof(shm_chat), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (chat == MAP_FAILED) { perror("mmap"); return ERROR; }

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
    draw_input("");
    pthread_mutex_unlock(&screen_lock);

    add_user();

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
                if (strcmp(input, "/quit") == 0) { running = 0; break; }
                post_msg(input, 0);
                len = 0; input[0] = '\0';
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

    remove_user();
    pthread_mutex_lock(&chat->lock);
    pthread_cond_broadcast(&chat->cond);
    pthread_mutex_unlock(&chat->lock);
    pthread_join(tid, NULL);

    munmap(chat, sizeof(shm_chat));
    close(fd);
    endwin();
    return SUCCESS;
}
