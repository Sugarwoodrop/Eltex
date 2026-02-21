#include "filemanager.h"

int main(){
    int fd;
    int error;
    fd = create_file("output.txt");
    if(fd == ERROR){
        perror("FD not create");
        return ERROR;
    }
    error = write_in_file(fd, "String from file");
    if(error == ERROR){
        return ERROR;
    }
    error = read_in_file(fd);
    if(error == ERROR){
        return ERROR;
    }
    return SUCCESS;
}
