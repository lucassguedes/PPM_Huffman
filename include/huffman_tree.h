#ifndef __HUFFMAN_TREE_H__
#define __HUFFMAN_TREE_H__
#include <stdlib.h>


typedef struct Node{
    int counter;
    struct Node* left_child;
    struct Node* right_child;   
}Node;

int compare_nodes(const void* a, const void* b);

Node* create_tree(int* counters, int n);




#endif