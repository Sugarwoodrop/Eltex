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

void SwapByte(int* numberFirst, int numberSecond){
    ToBitInt(*numberFirst);
    int* first = numberFirst;
    char* second = (char*)first + 2; 
    *second = (char)numberSecond;
    ToBitInt(*numberFirst);
}

int main(){
    int numberFirst;
    int numberSecond;

    scanf("%d", &numberFirst);
    scanf("%d", &numberSecond);

    if(numberSecond > 255 || numberSecond < 0){
        printf("Bad number\n");
        return ERROR;
    }
    SwapByte(&numberFirst, numberSecond);
    
    return SUCCES;
}
