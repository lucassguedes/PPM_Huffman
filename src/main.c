#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"
#include "hash_map.h"
#include "utils.h"

char utf8_ascii_table[256][256];

void show_tree(Node* root, int ntab){
	if(root == NULL){
		return;
	}
	for(int i = 0; i < ntab; i++)printf("\t");
	printf("Node - (counter: %d, symbol: %s)\n", root->symbol.counter, root->symbol.repr);
	for(int i = 0; i < ntab + 1; i++)printf("\t");
	printf("Children:\n");
	show_tree(root->left_child, ntab + 2);
	show_tree(root->right_child, ntab + 2);
	return;
}

int compare_symbols(const void * a, const void * b){
	int counter_cmp = ((Symbol*)a)->counter - ((Symbol*)b)->counter;

	if(!counter_cmp){
		return ((Symbol*)a)->repr[0] - ((Symbol*)b)->repr[0];
	}

	return counter_cmp;
}

int main(int argc, char** argv){

	
	// if(argc < 3){
	// 	printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
	// 	return -1;
	// }
	
	// initialize_convertion_table(utf8_ascii_table);
	
	// format_file(argv[1], argv[2], utf8_ascii_table);

	
	const int table_size = 50000; 

	/*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
	ContextInfo contexts[2];

	/*A tabela k=-1 inicialmente deve conter todos os símbolos do alfabeto, com contadores iguais a 1*/
	contexts[0].n_symb = 27;
	contexts[0].symb_table = (Item**)malloc(sizeof(Item*)*table_size);

	for(int i = 0; i < contexts[0].n_symb; i++) contexts[0].symb_table[i] = NULL;


	/*Adicionando símbolos do alfabeto*/
	Symbol s;
	s.repr = (char*)malloc(sizeof(char)*2);
	strcpy(s.repr, " ");
	s.counter = 1;
	
	add_item(contexts[0].symb_table, s);

	strcpy(s.repr, "a");
	for(int i = 0; i < 26; i++){
		add_item(contexts[0].symb_table, s);
		s.repr[0]++;
	}

	/*Extraindo símbolos da tabela de símbolos - esta operação é feita sempre que a árvore
	  precisa ser reconstruída.
	*/
	Symbol** all_symbols = extract_symbols(&contexts[0], table_size);
	HuffmanTree* tree = create_tree(all_symbols, 27);
	/*Atualiza os códigos dos símbolos de acordo com a estrutura da árvore*/
	set_codes(tree, all_symbols, 27);

	return 0;
}
