#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__
#include <string.h>
#include <math.h>
#include "huffman_tree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define A 0.6180339887
#define M pow(2, 14)


/*Representa um item de hash map (lista encadeada de símbolos)*/
typedef struct Item{
    struct Item* next;
    Symbol* value;
}Item;

int     hash(char* key);
void    add_item(Item* map[], Symbol symb);
void    remove_item(Item* map[], char* key);
void    increment_item(Item* map[], char* key);
void    decrement_item(Item* map[], char* key);
Symbol* get_item(Item* map[], char* key);
void    show_map(Item* map[], int size);
void    destroy_map(Item* map[], int size);

typedef struct ContextInfo{
    int n_symb;
    int max_search_length; /*Comprimento máximo de bits a buscar antes de passar para a próxima tabela*/
    Item** symb_table;
    HuffmanTree* tree;
}ContextInfo;

/*Dada uma  lista  de  símbolos  "symbols" e uma árvore
  de Huffman "tree", atribui os códigos correspondentes
  a cada símbolo de acordo com a estrutura da árvore.*/
void set_codes(ContextInfo* ctx, Symbol** symbols, int n);

Symbol** extract_symbols(ContextInfo* ctx_info, int table_size);


#endif 