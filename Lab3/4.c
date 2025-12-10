#include <stdio.h>

#define SUCCESS 0
#define ERROR -1
#define N 100

char* Search(char* fisrtString, char* secondString){
    int sizeEquel = 0;
    for(int i = 0; i < N; i++){
        if(fisrtString[i] == secondString[sizeEquel]){
            sizeEquel++;
        }
        else{
            sizeEquel = 0;
        }
        if(secondString[sizeEquel] == '\0' || secondString[sizeEquel] == '\n'){
            return fisrtString + i - sizeEquel + 1;
        }
        if(fisrtString[i] == '\0'){
            return NULL;
        }
    }
    return NULL;
}

int main(){
    char string[N];
    fgets(string, N, stdin);
    char stringSecond[N];
    fgets(stringSecond, N, stdin);

    char* answer = Search(string, stringSecond);
    if(answer == NULL){
        printf("Нету данной подстроки в строке\n");
        return SUCCESS;
    }
    int i = 0;
    while(1){
        if(answer[i] == '\0'){
            break;
        }
        printf("%c", answer[i]);
        i++;
    }
    return SUCCESS;
}
