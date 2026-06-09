#ifndef PROTOCOL_H
#define PROTOCOL_H

#define KEY_PATH "/tmp"
#define KEY_ID   'C'

#define MAX_NAME    32
#define MAX_TEXT    256
#define MAX_CLIENTS 32
#define MAX_HISTORY 200

enum {CMD_JOIN = 1, 
      CMD_MSG = 2, 
      CMD_LEAVE = 3 
};

enum {N_MSG = 1, 
      N_NAMES = 2, 
      N_SYS = 3 
};

struct req {
    long mtype;
    int  cmd;
    int  reply_qid;       
    char name[MAX_NAME];
    char text[MAX_TEXT];
};

struct notif {
    long mtype;
    int  kind;
    char name[MAX_NAME];
    char text[MAX_TEXT];
};

#endif 
