#ifndef __HUFFMAN_TREE_H__
#define __HUFFMAN_TREE_H__
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef struct Code{
    uint64_t value;
    int length;
}Code;

typedef struct Symbol{
    char* repr;
    int counter;
    Code code;
}Symbol;

/*Nó da árvore de Huffman*/
typedef struct Node{
    Symbol symbol;
    bool is_leaf;
    struct Node* parent;
    struct Node* left_child;
    struct Node* right_child;   
}Node;

/*Árvore de Huffman*/
typedef struct HuffmanTree{
    Node* root;
    Node** leafs;
}HuffmanTree;

int compare_nodes(const void* a, const void* b);

HuffmanTree* create_tree(Symbol** symbols, int n);

bool is_left_child(Node* node);
bool is_right_child(Node* node);

void destroy_tree(HuffmanTree* root);
void destroy_graph(Node* root);



#endif