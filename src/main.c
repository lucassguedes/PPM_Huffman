#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"
#include "hash_map.h"
#include "utils.h"
#include "ppm.h"

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

	
	if(argc < 3){
		printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
		return -1;
	}
	
	// initialize_convertion_table(utf8_ascii_table);
	
	// format_file(argv[1], argv[2], utf8_ascii_table);

	compress(argv[1], argv[2]);



	return 0;
}
