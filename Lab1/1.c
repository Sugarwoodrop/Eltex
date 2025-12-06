#include <stdio.h>

#define SUCCES 0
#define ERROR -1

void ToBitInt(int number){
    int count =0;
    for(int i =(int)sizeof(int) * 8 - 1; i >= 0; i--){
        if(count == 8){
            printf(" ");
            count = 0;
        }
        if( ((number>>i) & 1) == 0){
            printf("0");
            count++;
            continue;
        }
        printf("1");
        count++;
    }
    printf("\n");
}

int main(){
    int number;
    scanf("%d", &number);
    if(number < 0){
        printf("Bad number\n");
        return ERROR;
    }
    ToBitInt(number);
    
    return SUCCES;
}
