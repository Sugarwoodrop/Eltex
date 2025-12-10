#include <stdio.h>

#define SUCCESS 0
#define ERROR -1
#define N 10

int main(){
    int array[N];
    for(int i = 0; i < N; i++){
        *(array+i) = i+1;
    }
    for(int i = 0; i < N; i++){
        printf("%d ", *(array+i));
    }
    printf("\n");
    return SUCCESS;
}
