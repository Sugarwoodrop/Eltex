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
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == ERROR) {
        perror("shm_open");
        exit(ERROR);
    }
    if (ftruncate(fd, sizeof(struct region)) == ERROR) {
        perror("ftruncate");
        exit(ERROR);
    }

    struct region *r = mmap(NULL, sizeof(struct region),
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (r == MAP_FAILED) {
        perror("mmap");
        exit(ERROR);
    }

    sem_init(&r->sem_srv, 1, 0);
    sem_init(&r->sem_cli, 1, 0);

    strcpy(r->buf, "Hi!");
    sem_post(&r->sem_srv);

    sem_wait(&r->sem_cli);
    printf("Server received: %s\n", r->buf);

    sem_destroy(&r->sem_srv);
    sem_destroy(&r->sem_cli);
    munmap(r, sizeof(struct region));
    close(fd);
    shm_unlink(SHM_NAME);

    return SUCCESS;
}
