#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SUCCESS 0
#define ERROR  -1
#define SHM_NAME "/shm_demo"
#define MAX_MSG  256

struct region {
    sem_t sem_srv;
    sem_t sem_cli;
    char  buf[MAX_MSG];
};

int main() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == ERROR) {
        perror("shm_open (запустите сначала сервер)");
        exit(ERROR);
    }

    struct region *r = mmap(NULL, sizeof(struct region),
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (r == MAP_FAILED) {
        perror("mmap");
        exit(ERROR);
    }

    sem_wait(&r->sem_srv);
    printf("Client received: %s\n", r->buf);

    strcpy(r->buf, "Hello!");
    sem_post(&r->sem_cli);

    munmap(r, sizeof(struct region));
    close(fd);

    return SUCCESS;
}
