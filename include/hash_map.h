#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__
#include <string.h>
#include <math.h>
#include "huffman_tree.h"
#include "utils.h"
#include <stdio.h>

#define A 0.6180339887
#define M pow(2, 14)


/*Representa um item de hash map (lista encadeada de símbolos)*/
typedef struct Item{
    struct Item* next;
    Symbol* value;
}Item;

int     hash(char* key);
void    add_item(Item* map[], Symbol symb);
void    remove_item(Item* map[], int size, char* key);
void    increment_item(Item* map[], int size, char* key);
void    decrement_item(Item* map[], int size, char* key);
Symbol* get_item(Item* map[], char* key);
void    show_map(Item* map[], int size);
void    destroy_map(Item* map[], int size);


typedef struct ContextInfo{
    int n_symb;
    Item** symb_table;
    HuffmanTree* tree;
}ContextInfo;

Symbol** extract_symbols(ContextInfo* ctx_info, int table_size);


#endif 