#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define ERROR -1
#define SUCCESS 0
#define MAX_ATTEMPTS_WRITE 20

int create_file(char* filename);
int write_in_file(int fd, char* text);
int read_in_file(int fd);

#endif
