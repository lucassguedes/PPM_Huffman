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
Node* create_tree(Symbol* symbols, int n){

    /*Armazenando os contadores em uma lista de nós.*/
    Node** nodes = (Node**)malloc(sizeof(Node*)*n);

    for(int i = 0; i < n; i++){
        nodes[i] = (Node*)malloc(sizeof(Node));
        nodes[i]->symbol = symbols[i];
        nodes[i]->left_child = nodes[i]->right_child = NULL;
    }

    /*Ordenando os nós em ordem crescente de seus contadores.*/
    qsort(nodes, n, sizeof(Node*), compare_nodes);

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

        /*Adicionando nós filhos do novo nó*/
        if(nodes[0]->symbol.counter < nodes[1]->symbol.counter){
            new_node->left_child = nodes[1];
            new_node->right_child = nodes[0];
        }else{
            new_node->left_child = nodes[0];
            new_node->right_child = nodes[1];
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

    return new_node;
}

int compare_nodes(const void* a, const void* b){
	return ((*(Node**)a)->symbol.counter - (*(Node**)b)->symbol.counter);
}