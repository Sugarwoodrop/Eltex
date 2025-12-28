#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SUCCESS 0

typedef struct nodes {
    char name[10];
    char second_name[10];
    char tel[10];
    struct nodes* next;
    struct nodes* prev;
}node;

typedef struct abonents{
    int size;
    node* first;
    node* last;
}abonent;

void addAbonent(abonent* abonents){

    node newAbonent;
    char buf[30];

    fgets(buf, sizeof(buf), stdin);

    if (sscanf(buf, "%9s %9s %9s",
            newAbonent.name,
            newAbonent.second_name,
            newAbonent.tel) != 3) {
        printf("Неверный формат ввода\n\n");
        return;
    }
    if(abonents->size == 0){
        abonents->first = malloc(sizeof(node));
        if(abonents->first == NULL){
            printf("ERROR malloc in addAbonent");
            return;
        }
        abonents->last = abonents->first;
    }
    else{
        abonents->last->next = malloc(sizeof(node));
        if(abonents->last->next == NULL){
            printf("ERROR malloc in addAbonent");
            return;
        }
        abonents->last->next->prev = abonents->last;
        abonents->last = abonents->last->next;
    }
    abonents->size++;
    memcpy(abonents->last->name, newAbonent.name, 10);
    memcpy(abonents->last->second_name, newAbonent.second_name, 10);
    memcpy(abonents->last->tel, newAbonent.tel, 10);
}

void deleteAbonent(abonent* abonents){
    int numberAbonent;
    char buf[32];
    fgets(buf, sizeof(buf), stdin);

    if (sscanf(buf, "%d", &numberAbonent) != 1) {
        printf("Не коректный номер абонента\n\n");
        return;
    }

    if(abonents->size > 100 || numberAbonent <=0){
        printf("Не коректный номер абонента\n\n");
        return;
    }
    numberAbonent--;
    node* temp = abonents->first;
    for(int i = 0; i < numberAbonent; i++){
        temp = temp->next;
    }

    if(temp == abonents->first){
        abonents->first = temp->next;
        if(temp->next != NULL){
            temp->next->prev = NULL;
        }
    }
    else if(temp == abonents->last){
        abonents->last = temp->prev;
        temp->prev->next = NULL;
    }
    else{ 
        temp->prev->next = temp->next;
        temp->next->prev = temp->prev;
    }
    free(temp);
}

void printfAbonent(node abonent, int i){
    printf("%d) %s %s, %s\n", i, abonent.name, abonent.second_name, abonent.tel);
}

void searcheAbonent(abonent* abonents){
    char name[10];
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    node* temp = abonents->first;
    for(int i = 0; i < abonents->size; i++){
        if(strcmp(temp->name, name) == 0){
            printfAbonent(*temp, i+1);
            return;
        }
        if(temp != abonents->last){
            temp = temp->next;
        }
    }
    printf("Не найдено\n\n");
}

void printfAllAbonents(abonent* abonents){
    node* temp = abonents->first;

    for(int i = 0; i < abonents->size; i++){
        printfAbonent(*temp, i+1);
        if(temp != abonents->last){
            temp = temp->next;
        }
    }
    printf("\n\n");
}

int main(){
    void (*menu_fn[])(abonent*) = {addAbonent, deleteAbonent, searcheAbonent, printfAllAbonents};
    abonent abonents;
    abonents.size = 0;
    abonents.first = NULL;
    abonents.last = NULL;
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
        menu_fn[Numfunc-1](&abonents);
    }

    return SUCCESS;
}
