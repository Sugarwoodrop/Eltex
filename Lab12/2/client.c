#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR  -1
#define FIFO_NAME "myfifo"
#define MAX_LENGTH 1024

int main() {
    int fd = open(FIFO_NAME, O_RDONLY);
    if (fd == ERROR) {
        perror("open");
        exit(ERROR);
    }

    char buf[MAX_LENGTH];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n == ERROR) {
        perror("read");
        close(fd);
        exit(ERROR);
    }
    buf[n] = '\0';
    printf("Client received: %s\n", buf);

    close(fd);

    if (unlink(FIFO_NAME) == ERROR) {
        perror("unlink");
        exit(ERROR);
    }

    return SUCCESS;
}
