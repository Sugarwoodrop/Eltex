#ifndef PANEL_H
#define PANEL_H

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/limits.h>
#include <ncurses.h>
#include <stdlib.h>

#define ERROR -1
#define SUCCESS 0

struct Panel{
    char path[PATH_MAX];
    char** dir;
    char** file;
    int count_dir;
    int count_file;            
    int selected; 
};

int initPanel(struct Panel* panel);
    
int openDir(struct Panel* panel);

void next_file(struct Panel* panel);
void prev_file(struct Panel* panel);
void delitePanel(struct Panel* panel);
#endif
