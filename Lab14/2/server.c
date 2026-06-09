#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#include "shm_chat.h"

#define SUCCESS 0
#define ERROR  -1

static shm_chat *chat = NULL;
static int fd = ERROR;

static void cleanup(int sig) {
    (void)sig;
    if (chat && chat != MAP_FAILED) {
        pthread_mutex_destroy(&chat->lock);
        pthread_cond_destroy(&chat->cond);
        munmap(chat, sizeof(shm_chat));
    }
    if (fd != ERROR) close(fd);
    shm_unlink(SHM_NAME);
    printf("\nServer stopped, shared segment removed.\n");
    exit(SUCCESS);
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == ERROR) { perror("shm_open"); exit(ERROR); }
    if (ftruncate(fd, sizeof(shm_chat)) == ERROR) { perror("ftruncate"); exit(ERROR); }

    chat = mmap(NULL, sizeof(shm_chat), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (chat == MAP_FAILED) { perror("mmap"); exit(ERROR); }

    memset(chat, 0, sizeof(shm_chat));

    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&chat->lock, &ma);
    pthread_mutexattr_destroy(&ma);

    pthread_condattr_t ca;
    pthread_condattr_init(&ca);
    pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&chat->cond, &ca);
    pthread_condattr_destroy(&ca);

    chat->total = 0;
    chat->user_count = 0;

    printf("Chat server: shared segment ready. (Ctrl+C to stop)\n");
    printf("Clients communicate through it directly.\n");

    long seen = 0;
    pthread_mutex_lock(&chat->lock);
    while (1) {
        while (chat->total == seen)
            pthread_cond_wait(&chat->cond, &chat->lock);
        while (seen < chat->total) {
            chat_msg *m = &chat->msgs[seen % MAX_MSGS];
            if (m->is_sys)
                printf("[server] *** %s %s ***\n", m->name, m->text);
            else
                printf("[server] %s: %s\n", m->name, m->text);
            fflush(stdout);
            seen++;
        }
    }

    pthread_mutex_unlock(&chat->lock);
    return SUCCESS;
}
