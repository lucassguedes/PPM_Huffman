#include "hash_map.h"

void add_item(Item* map[], Symbol symb){
    int index = hash(symb.repr);

    Item* new_item = (Item*)malloc(sizeof(Item));
    new_item->value = (Symbol*)malloc(sizeof(Symbol));
    new_item->value->code = symb.code;
    new_item->value->counter = symb.counter;
    new_item->value->repr = (char*)malloc(sizeof(char)*(strlen(symb.repr) + 1));
    new_item->next = NULL;
    strcpy(new_item->value->repr, symb.repr);

    if(map[index] != NULL){
        Item* it = map[index];

        while(it->next != NULL){
            it = it->next;
        }

        it->next = new_item;
    }else{
        map[index] = new_item;
    }
}

Symbol* get_item(Item* map[], char* key){
    int index = hash(key);

    Item* it = map[index];

    while(strcmp(it->value->repr, key)){
        it = it->next;
    }

    return it->value; 
}

void show_map(Item* map[], int size){
    int counter = 1;
    for(int i = 0; i < size; i++){
        Item* it = map[i];
        while(it != NULL){
            printf("Item %d:\n", counter);
            printf("\tSymbol: %s\n", map[i]->value->repr);
            printf("\tCounter: %d\n", map[i]->value->counter);
            it = it->next;
            counter++;
        }
    }
}

void destroy_map(Item* map[], int size){
	for(int i = 0; i < size; i++){
		Item* it = map[i];
		Item* next;
		while(it != NULL){
			next = it->next;

			free(it->value->repr);
			free(it->value);
			free(it);
			it = next;
		}
	}
}


int hash(char* key){
	/**Interpretando a string como um número natural.*/
	int n = strlen(key);
	int k = 0;
	for(int i = 0; i < n; i++){
		k += (int)key[i] * pow(128, n - i - 1);
	}

	/*Aplicando o método da multiplicação*/
	return (int)floor(M*(k*A - floor(k*A)));
}