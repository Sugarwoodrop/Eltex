#include <stdio.h>
#include "add.h"
#include "div.h"
#include "mul.h"
#include "sub.h"

#define SUCCESS 0

int main(){
    void(*menu_fn[])(int, int) = {add, sub, mul, div};
    
    while(1){
    printf("1) Сложение\n2) Вычитание\n3) Умножение\n4) Деление\n5) Выход\n");    
    int Numfunc;
    if (scanf("%d", &Numfunc) != 1) 
        continue;

    if(Numfunc == 5){
        return SUCCESS;
    }
    if(Numfunc < 0 || Numfunc > 5){
        continue;
    }
    int first_number;
    int second_number;

    scanf("%d", &first_number);
    scanf("%d", &second_number);

        menu_fn[Numfunc-1](first_number, second_number);
    }

    return SUCCESS;
}
