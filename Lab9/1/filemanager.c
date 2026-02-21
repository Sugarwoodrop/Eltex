#include "filemanager.h"

int create_file(char* filename){
    int fd;
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    return fd;
}

int write_in_file(int fd, char* text){
    ssize_t  write_bytes = 0;
    size_t  all_bytes = strlen(text);
    int recording_attempts = 0;
    while(1){
        write_bytes += write(fd, text, all_bytes - write_bytes);
        if(write_bytes < 0){
            perror("WRITE ERROR");
            return ERROR;
        }
        if((size_t)write_bytes == all_bytes){
            return SUCCESS;
        }
        recording_attempts++;
        if(recording_attempts == MAX_ATTEMPTS_WRITE){
            perror("WRITE NOT RECODING ALL");
            return ERROR;
        }
        usleep(20000);
    }
}

int read_in_file(int fd){
    off_t size = lseek(fd, 0, SEEK_END);
    if(size == ERROR){
        perror("lseek ERROR");
        return ERROR;
    }
    int error;
    for (off_t i = size - 1; i >= 0; i--) {
        error = lseek(fd, i, SEEK_SET);
        if (lseek(fd, i, SEEK_SET) == -1) {
            perror("lseek");
            return ERROR;
        }

        char c;
        error = read(fd, &c, 1);
        if (error == ERROR) {
            perror("read");
            return ERROR;
        }

        error = write(1, &c, 1);
        if (error == ERROR) {
            perror("write");
            return ERROR;
        }
    }
    write(1, "\n", 1);
    return SUCCESS;
}
