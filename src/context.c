#include "context.h"

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

void remove_item(Item* map[], char* key){
    const int index = hash(key);
    
    Item* it = map[index];
    Item* prev = NULL;
    while(it != NULL){
        if(!strcmp(it->value->repr, key)){//Se encontrou o símbolo
            if(prev != NULL){
                prev->next = it->next;
            }else{
                map[index] = it->next;
            }

            free(it->value->repr);
            free(it->value);
            free(it);
            break;
        }
        prev = it;
        it = it->next;
    }
}

void decrement_item(Item* map[], char* key){
    const int index = hash(key);
    
    Item* it = map[index];
    Item* prev = NULL;
    while(it != NULL){
        if(!strcmp(it->value->repr, key)){//Se encontrou o símbolo
            it->value->counter--;

            if(!it->value->counter){
                if(prev != NULL){
                    prev->next = it->next;
                }else{
                    map[index] = it->next;
                }

                free(it->value->repr);
                free(it->value);
                free(it);
            }
            break;
        }
        prev = it;
        it = it->next;
    }
}

void increment_item(Item* map[], char* key){
    const int index = hash(key);
    Item* it = map[index];
    while(it != NULL){
        if(!strcmp(it->value->repr, key)){//Se encontrou o símbolo
            it->value->counter++;
            break;
        }
        it = it->next;
    }
}

Symbol* get_item(Item* map[], char* key){
    int index = hash(key);

    Item* it = map[index];

    while(it != NULL){
        if(!strcmp(it->value->repr, key))
        {
            return it->value;
        }
        it = it->next;
    }

    return NULL; 
}

void show_map(Item* map[], int size){
    int counter = 1;
    char buffer[33];
    for(int i = 0; i < size; i++){
        Item* it = map[i];
        while(it != NULL){
            printf("Item %d:\n", counter);
            printf("\tSymbol: %s\n", it->value->repr);
            printf("\tCounter: %d\n", it->value->counter);
            get_bin_str(it->value, buffer);
            printf("\tCode: %s\n", buffer);
            it = it->next;
            counter++;
        }
    }
}

Symbol** extract_symbols(ContextInfo* ctx_info, int table_size){
    Symbol** symbols = (Symbol**)malloc(sizeof(Symbol*)*ctx_info->n_symb);

    int added = 0;
    for(int i = 0; i < table_size; i++){
        Item* it = ctx_info->symb_table[i];

        while(it != NULL){
            symbols[added] = it->value;
            added++;
            it = it->next;
        }
    }

    return symbols;
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
        map[i] = NULL;
	}
    free(map);
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

int compare_symbols(const void * a, const void * b){
    Symbol* sa = *(Symbol**)a;
    Symbol* sb = *(Symbol**)b;
	int counter_cmp = sa->counter - sb->counter;
	if(!counter_cmp){
		return strcmp(sb->repr, sa->repr);
	}

	return counter_cmp;
}


void set_codes(ContextInfo* ctx){
    ctx->max_search_length = 0;

    const int n_leafs = ctx->n_symb;

    for(int i = 0; i < n_leafs; i++){
        Symbol* symbol = get_item(ctx->symb_table, ctx->tree->leafs[i]->symbol.repr);
        Node* leaf = ctx->tree->leafs[i];

        symbol->code.value = 0;
        symbol->code.length = 0;

        while(leaf != NULL){
            if(leaf->parent != NULL){
                // symbols[i]->code.value = symbols[i]->code.value << 1;
                if(is_left_child(leaf)){
                    // printf("1");
                    symbol->code.value |= (int)pow(2, symbol->code.length);
                }else{
                    // printf("0");
                }
                symbol->code.length++;
            }
            leaf = leaf->parent;
        }
        if(symbol->code.length > ctx->max_search_length){
            ctx->max_search_length = symbol->code.length;
        }
    }

}