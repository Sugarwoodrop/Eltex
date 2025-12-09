#include <stdio.h>

#define SUCCESS 0
#define ERROR -1

void PrintMatrix(int size){
    int numberLine = 1;
    for(int i = 1; i <= size*size; i++){
        printf("%d ", i);
        if(i == size*numberLine){
            printf("\n");
            numberLine++;
        } 
    }
}

int main(){
    int sizeMatrix = 0;
    scanf("%d", &sizeMatrix);

    if(sizeMatrix <= 0){
        printf("Error: Bad Number");
        return ERROR;
    }

    PrintMatrix(sizeMatrix);

    return SUCCESS;
}
