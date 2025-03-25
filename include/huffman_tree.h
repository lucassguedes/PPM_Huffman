#ifndef __HUFFMAN_TREE_H__
#define __HUFFMAN_TREE_H__
#include <stdlib.h>
#include <string.h>

typedef struct Symbol{
    char* repr;
    int counter;
}Symbol;

typedef struct Node{
    Symbol symbol;
    struct Node* left_child;
    struct Node* right_child;   
}Node;

int compare_nodes(const void* a, const void* b);

Node* create_tree(Symbol* counters, int n);




#endif