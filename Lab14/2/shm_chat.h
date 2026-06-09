#ifndef SHM_CHAT_H
#define SHM_CHAT_H

#include <pthread.h>

#define SHM_NAME    "/shm_chat"

#define MAX_NAME    32
#define MAX_TEXT    256
#define MAX_MSGS    256
#define MAX_USERS   32

typedef struct {
    char name[MAX_NAME];
    char text[MAX_TEXT];
    int  is_sys; 
} chat_msg;

typedef struct {
    pthread_mutex_t lock; 
    pthread_cond_t  cond;

    long     total;
    chat_msg msgs[MAX_MSGS];

    int  user_count;
    char users[MAX_USERS][MAX_NAME];
} shm_chat;

#endif
