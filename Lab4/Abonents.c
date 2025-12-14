#include <stdio.h>
#include <string.h>

#define SUCCESS 0

typedef struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
    char isdelete;
}abonent;

void addAbonent(abonent* abonents){

    abonent newAbonent;
    char buf[30];

    fgets(buf, sizeof(buf), stdin);

    if (sscanf(buf, "%9s %9s %9s",
           newAbonent.name,
           newAbonent.second_name,
           newAbonent.tel) != 3) {
        printf("Неверный формат ввода\n\n");
        return;
    }
    newAbonent.isdelete = 0;

    for(int i = 0; i < 100; i++){
        if(abonents[i].isdelete == 1){
            abonents[i] = newAbonent;
            printf("Абонент добавлен\n\n");
            return;
        }
    }
    printf("Нельзя больше 100 абонентов\n\n");
}

void deleteAbonent(abonent* abonents){
    int numberAbonent;
    char buf[32];
    fgets(buf, sizeof(buf), stdin);

    if (sscanf(buf, "%d", &numberAbonent) != 1) {
        printf("Не коректный номер абонента\n\n");
        return;
    }

    if(numberAbonent > 100 || numberAbonent <=0){
        printf("Не коректный номер абонента\n\n");
        return;
    }

    abonents[numberAbonent-1].isdelete = 1;
    memset(abonents[numberAbonent-1].name, '0', 10);
    memset(abonents[numberAbonent-1].second_name, '0', 10);
    memset(abonents[numberAbonent-1].tel, '0', 10);
}

void printfAbonent(abonent abonent, int i){
    printf("%d) %s %s, %s\n", i, abonent.name, abonent.second_name, abonent.tel);
}

void searcheAbonent(abonent* abonents){
    char name[10];
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    for(int i = 0; i < 100; i++){
        if(strcmp(abonents[i].name, name) == 0 && abonents[i].isdelete == 0){
            printfAbonent(abonents[i], i+1);
            return;
        }
    }
    printf("Не найдено\n\n");
}

void printfAllAbonents(abonent* abonents){
    for(int i = 0; i < 100; i++){
        if(abonents[i].isdelete == 0){
            printfAbonent(abonents[i], i+1);
        }
    }
    printf("\n\n");
}

int main(){
    void (*menu_fn[])(abonent*) = {addAbonent, deleteAbonent, searcheAbonent, printfAllAbonents};
    abonent abonents[100];
    for(int i = 0; i < 100; i++){
        abonents[i].isdelete = 1;
        memset(abonents[numberAbonent-1].name, '0', 10);
        memset(abonents[numberAbonent-1].second_name, '0', 10);
        memset(abonents[numberAbonent-1].tel, '0', 10);
    }
    while(1){
        printf("1) Добавить абонента\n2) Удалить абонента\n3) Поиск абонентов по имени\n4) Вывод всех записей\n5) Выход\n");
        
        char buf[32];
        fgets(buf, sizeof(buf), stdin);

        int Numfunc;
        if (sscanf(buf, "%d", &Numfunc) != 1) 
            continue;

        if(Numfunc == 5){
            return SUCCESS;
        }
        if(Numfunc < 0 || Numfunc > 5){
            continue;
        }
        menu_fn[Numfunc-1](abonents);
    }

    return SUCCESS;
}
