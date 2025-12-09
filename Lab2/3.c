#include <stdio.h>

#define SUCCESS 0
#define N 10

int main(){
    int matrix[N*N];
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            int number = 0;

            if(i<=j){
                number = 1;
            }

            matrix[(i*N)+j] = number;
        }
    }

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%d ", matrix[(i*N)+j]);
        }
        printf("\n");
    }

    return SUCCESS;
}
