#include <stdio.h>

#define SUCCES 0
#define ERROR -1

void PrintSizeBitOne(int number){
    int count =0;
    for(int i =(int)sizeof(int) * 8 - 1; i >= 0; i--){
        if( ((number>>i) & 1) == 1){
            count++;
        }
    }
    printf("%d\n", count);
}

int main(){
    int number;
    scanf("%d", &number);
    if(number < 0){
        printf("Bad number\n");
        return ERROR;
    }
    PrintSizeBitOne(number);
    
    return SUCCES;
}
