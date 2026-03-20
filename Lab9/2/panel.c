#include "panel.h"
#include <dirent.h>

int initPanel(struct Panel* panel){
    panel->selected = -1;
    panel->count_dir = 0;
    panel->count_file = 0;
    int error = openDir(panel);
    if(error == ERROR){
        return ERROR;
    }
    return SUCCESS;
}


int openDir(struct Panel* panel){
    DIR* dir;
    if(panel->selected == -1){
        dir = opendir(".");
    }
    else{
        dir = opendir(panel->dir[panel->selected]);
    }
    if(dir == NULL){
        perror("opendir");
        delitePanel(panel);
        return ERROR;
    }

    if (panel->selected == -1) {
        chdir(".");
    } else {
        chdir(panel->dir[panel->selected]);
    }
    
    delitePanel(panel);
    
    panel->count_dir = 0;
    panel->count_file = 0;
    panel->dir  = NULL;
    panel->file = NULL;
    if(getcwd(panel->path, PATH_MAX) == NULL){
        perror("getcwd initPanel");
        return ERROR;
    }

    struct dirent *ep;
    while((ep = readdir(dir)) != NULL){
        if(ep->d_type == DT_DIR){
            void* tmp = realloc(panel->dir, (panel->count_dir+1)* sizeof(char*));
            if (!tmp) {
                closedir(dir);
                delitePanel(panel);
                return ERROR;
            } 
            else {
                panel->dir = tmp;
            }   
            char *name = strdup(ep->d_name);
            if (!name){ 
                closedir(dir);
                delitePanel(panel);
                return ERROR;
            }
            panel->dir[panel->count_dir] = name;
            panel->count_dir++;
        }
        else{
            void* tmp = realloc(panel->file, (panel->count_file+1)* sizeof(char*));
            if (!tmp) {
                closedir(dir);
                delitePanel(panel);
                return ERROR;
            } 
            else {
                panel->file = tmp;
            }
            char *name = strdup(ep->d_name);
            if (!name){ 
                closedir(dir);
                delitePanel(panel);
                return ERROR;
            }
            panel->file[panel->count_file] = name;
            panel->count_file++;
        }
    }
    panel->selected = 0;
    closedir(dir);
    return SUCCESS;
}

void next_file(struct Panel* panel){
    panel->selected += 1;
    if(panel->selected >= panel->count_dir+panel->count_file){
        panel->selected = 0;
    }
}
void prev_file(struct Panel* panel){
    panel->selected -= 1;
    if(panel->selected < 0){
        panel->selected = panel->count_dir+panel->count_file-1;
    }
}

void delitePanel(struct Panel* panel){
    for(int i = 0; i < panel->count_dir; i++){
        free(panel->dir[i]);
    }
    for(int i = 0; i < panel->count_file; i++){
        free(panel->file[i]);
    }
    if(panel->count_dir != 0){
        free(panel->dir);
    }
    if(panel->count_file != 0){
        free(panel->file);
    }
}
