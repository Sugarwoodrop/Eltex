#include "shop.h"

int init_stall(stall* stall){
    srand(time(NULL));

    stall->stock = 900 + rand() % 200;
    if(pthread_mutex_init(&stall->mutex, NULL) != 0){
        perror("ошибка инициализации мютекса");
        return ERROR;
    }
    return SUCCESS;
}

int delete_stall(stall* stall){
    pthread_mutex_destroy(&stall->mutex);
    return SUCCESS;
}

int start_shopping(stall* stall, buyer* buyer){
    if(pthread_mutex_trylock(&stall->mutex) != 0){
        return 1;
    }
     if(stall->stock <= 0){
        pthread_mutex_unlock(&stall->mutex);
        return ERROR;
    }
    while (stall->stock > 0 && buyer->demand > 0)
    {
        stall->stock --;
        buyer->demand--;
    }
    pthread_mutex_unlock(&stall->mutex);
    return SUCCESS;
}

int store_replenishment(stall* stall){
    if(pthread_mutex_trylock(&stall->mutex) != 0){
        return 1;
    }
    stall->stock += 200;
    pthread_mutex_unlock(&stall->mutex);
    return SUCCESS;
}
