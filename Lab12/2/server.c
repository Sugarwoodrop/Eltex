#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SUCCESS 0
#define ERROR  -1
#define FIFO_NAME "myfifo"

int main() {
    if (mkfifo(FIFO_NAME, 0666) == ERROR && errno != EEXIST) {
        perror("mkfifo");
        exit(ERROR);
    }

    int fd = open(FIFO_NAME, O_WRONLY);  
    if (fd == ERROR) {
        perror("open");
        exit(ERROR);
    }

    const char *msg = "Hi!";
    if (write(fd, msg, strlen(msg)) == ERROR) {
        perror("write");
        close(fd);
        exit(ERROR);
    }

    close(fd);
    return SUCCESS;
}
