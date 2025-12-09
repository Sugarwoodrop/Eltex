#include <stdio.h>

#define SUCCESS 0
#define N 5

int main(){
    int matrix[N*N];
    int number = 1;

    int top = 0, bottom = N-1;
    int left = 0, right = N-1;

    while (top <= bottom && left <= right) {
        for (int j = left; j <= right; j++)
            matrix[top*N + j] = number++;
        top++;

        for (int i = top; i <= bottom; i++)
            matrix[i*N + right] = number++;
        right--;

        for (int j = right; j >= left; j--)
            matrix[bottom*N + j] = number++;
        bottom--;

        for (int i = bottom; i >= top; i--)
            matrix[i*N + left] = number++;
        left++;
    }

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%d ", matrix[(i*N)+j]);
        }
        printf("\n");
    }

    return SUCCESS;
}
