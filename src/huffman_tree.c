#include "huffman_tree.h"
#include <stdio.h>
/**
 * Nome: create_tree
 * 
 * Parametros:
 *      @counters - contadores dos símbolos
 *      @n        - quantidade de contadores
 * 
 * Descrição: Retorna uma árvore de Huffman construída
 * com base nos contadores dos símbolos.
 */
HuffmanTree* create_tree(Symbol* symbols, int n){
    HuffmanTree* tree = (HuffmanTree*)malloc(sizeof(HuffmanTree));

    /*Armazenando os contadores em uma lista de nós.*/
    Node** nodes = (Node**)malloc(sizeof(Node*)*n);

    tree->leafs = (Node**)malloc(sizeof(Node*)*n);

    for(int i = 0; i < n; i++){
        nodes[i] = (Node*)malloc(sizeof(Node));
        nodes[i]->symbol.counter = symbols[i].counter;
        nodes[i]->symbol.repr = (char*)malloc(sizeof(char)*(strlen(symbols[i].repr) + 1));
        strcpy(nodes[i]->symbol.repr, symbols[i].repr);
        nodes[i]->parent = nodes[i]->left_child = nodes[i]->right_child = NULL;
        nodes[i]->symbol.code.value = 0;
        nodes[i]->symbol.code.length = 0;
        nodes[i]->is_leaf = true;
    }

    /*Ordenando os nós em ordem crescente de seus contadores.*/
    qsort(nodes, n, sizeof(Node*), compare_nodes);

    for(int i = 0; i < n; i++) tree->leafs[i] = nodes[i];

    /**
     * Enquanto houver mais de um nó na lista de nós, construa a árvore de Huffman.
     */
    Node* new_node = NULL;
    while(n > 1){
        new_node = (Node*)malloc(sizeof(Node));

        /**
         * O novo nó recebe a soma dos contadores dos dois nós com os
         * menores contadores.
         */
        char tmp[100];

        sprintf(tmp, "%s,%s", nodes[0]->symbol.repr, nodes[1]->symbol.repr);

        new_node->symbol.repr = (char*)malloc(sizeof(char)*(strlen(tmp) + 1));
        strcpy(new_node->symbol.repr, tmp);
        new_node->symbol.counter = nodes[0]->symbol.counter + nodes[1]->symbol.counter;

        nodes[0]->parent = nodes[1]->parent = new_node;
        /*Adicionando nós filhos do novo nó*/
        if(nodes[0]->symbol.counter < nodes[1]->symbol.counter){
            if(nodes[0]->is_leaf && !nodes[1]->is_leaf){
                new_node->left_child = nodes[0];
                new_node->right_child = nodes[1];
            }else{
                new_node->left_child = nodes[1];
                new_node->right_child = nodes[0];
            }
        }else{
            if(nodes[1]->is_leaf && !nodes[0]->is_leaf){
                new_node->left_child = nodes[1];
                new_node->right_child = nodes[0];
            }else{
                new_node->left_child = nodes[0];
                new_node->right_child = nodes[1];
            }
            
        }

        /**Subescreve os dois nós menores e adiciona o novo nó na lista de nós*/
        for(int i = 0; i < n - 2; i++){
            nodes[i] = nodes[i+2];
        }

        nodes[n-2] = new_node;
        n--;

        /*Ordenando nós*/
        qsort(nodes, n, sizeof(Node**), compare_nodes);
        
    }

    
    free(nodes);
    
    new_node->parent = NULL;
    tree->root = new_node;

    return tree;
}

bool is_left_child(Node* node){
    return node->parent != NULL && node->parent->left_child == node;
}

bool is_right_child(Node* node){
    return node->parent != NULL && node->parent->right_child == node;
}

void set_codes(HuffmanTree* tree, Symbol* symbols, int n){
    /*Consideramos  que os símbolos das folhas da árvore
      estão na mesma ordem dos símbolos em symbols, isto
      é, em ordem crescente de seus contadores.
    */
    for(int i = 0; i < n; i++){
        /*Encontra o código correspondente*/
        Node* node = tree->leafs[i];

        symbols[i].code.value = 0;
        symbols[i].code.length = 0;
        while(node != NULL){
            if(node->parent != NULL){
                // symbols[i].code.value = symbols[i].code.value << 1;
                if(is_left_child(node)){
                    symbols[i].code.value |= (int)pow(2, symbols[i].code.length);
                }
                symbols[i].code.length++;
            }
            

            node = node->parent;
        }
    }
}

void destroy_graph(Node* root){
    if(root == NULL){
        return;
    }

    destroy_graph(root->left_child);
    destroy_graph(root->right_child);

    free(root->symbol.repr);
    free(root);
}

void destroy_tree(HuffmanTree* tree){
    destroy_graph(tree->root);
    free(tree->leafs);
    free(tree);
}

int compare_nodes(const void* a, const void* b){
	return ((*(Node**)a)->symbol.counter - (*(Node**)b)->symbol.counter);
}