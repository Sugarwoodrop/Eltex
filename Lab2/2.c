#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR -1

int main(){
    int* buffer = malloc(sizeof(int)*1); 
    if(buffer == NULL){
        printf("ERROR malloc");
        return ERROR;
    }
    int numberElements = 0;

    while(1){
        if(scanf("%d", buffer+numberElements) == EOF){ //Закончить ввод Enter потом Ctr+D
            break;
        }
        numberElements++;
        int* tmp = realloc(buffer ,sizeof(int)*(numberElements+1));
        if(tmp == NULL){
            printf("ERROR realloc");
            free(buffer);
            return ERROR;
        }
        buffer = tmp;
    }
    printf("\n");
    for(int i = 0; i < numberElements; i++){
        printf("%d ", buffer[numberElements-1-i]);
    }
    printf("\n");
    
    free(buffer);
    return SUCCESS;
}
