#include <pthread.h>
#include "buyer.h"
#include <stdio.h>

#define ERROR -1
#define SUCCESS 0

typedef struct{
    int stock;
    pthread_mutex_t mutex;
}stall;

int init_stall(stall* stall);
int delete_stall(stall* stall);
int start_shopping(stall* stall, buyer* buyer);

int store_replenishment(stall* stall);
