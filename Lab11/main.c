#include "shop.h"
#include <errno.h>
#include <unistd.h>


#define NUM_BUYERS 3
#define NUM_STALL 5

typedef struct {
    stall *stalls;
    int id;
} buyer_args;

void* buyers_func(void *arg){
    buyer_args *args = (buyer_args *) arg;
    stall* stalls = args->stalls;
    buyer data;
    init_buyer(&data);
    while(data.demand > 0){
        int entered = 0;
        int stall_empty = 0;
        int num_stall = -1;
        
        for(int i = 0; i < NUM_STALL; i++){
            if(start_shopping(&stalls[i], &data) == SUCCESS){
                entered = 1;
                if(data.demand > 0){
                    stall_empty = 1;
                }
                num_stall = i;
                break;
            }
        }
        if(!entered){
            printf("Поток покупатель %d, не взял ни одного товара и уснул на 1 сек\n", args->id);
            sleep(1);
        }
        if(stall_empty){
            printf("Поток покупатель %d, опусташил магазни %d и уснул на 2 сек. Осталось %d\n", args->id, num_stall, data.demand);
            sleep(2);
        }
    }
    printf("Поток покупатель %d насытелся\n", args->id);
    return NULL;
}

void* loader_func(void *arg){
    stall* stalls = (stall*) arg;
    srand(time(NULL));
    while(1){
        int num_stull = rand() % 5;
        if(store_replenishment(&stalls[num_stull]) != SUCCESS){
            printf("Поток погрузчика, не смог зайти в магазин %d\n", num_stull);
            continue;
        }
        else{
            printf("Поток погрузчика, пополнил магазин %d, и уснул на 1 сек\n", num_stull);
            sleep(1);
        }
    }
    return NULL;
}

int main(){
    pthread_t buyers[NUM_BUYERS];
    pthread_t loader;
    stall *stalls = malloc(sizeof(stall)*NUM_STALL);
    buyer_args args[NUM_BUYERS];
    for(int i = 0; i < NUM_STALL; i++){
        init_stall(&stalls[i]);
    }
    for(int i = 0; i < NUM_BUYERS; i++){
        args[i].stalls = stalls;
        args[i].id = i;
        if(pthread_create(&buyers[i], NULL, buyers_func, &args[i]) != SUCCESS){
            perror("Ошибка создания потока");

            return ERROR;
        }
    }
    if(pthread_create(&loader, NULL, loader_func, stalls) != SUCCESS){
        perror("Ошибка создания потока");

        return ERROR;
    }

    for (int i = 0; i < NUM_BUYERS; i++) {
        if (pthread_join(buyers[i], NULL) != 0) {
            perror("pthread_join buyer");
        }
    }
    pthread_cancel(loader);
    pthread_join(loader, NULL);
    for(int i = 0; i < NUM_STALL; i++){
        delete_stall(&stalls[i]);
    }
    free(stalls);
    return SUCCESS;
}
