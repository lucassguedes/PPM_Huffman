#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"

char utf8_ascii_table[256][256];

void get_bin_str(Symbol* symb, char buffer[]){
	uint64_t value = symb->code.value;
	const int length = symb->code.length;
	buffer[length] = '\0';
	for(int i = 0; i < length; i++){
		if((value & 1) == 1){
			buffer[length - i - 1] = '1';
		}else{
			buffer[length - i - 1] = '0';
		}
		value >>= 1;
	}
}

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
	return ((Symbol*)a)->counter - ((Symbol*)b)->counter;
}

int main(int argc, char** argv){

	
	// if(argc < 3){
	// 	printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
	// 	return -1;
	// }
	
	// initialize_convertion_table(utf8_ascii_table);
	
	// format_file(argv[1], argv[2], utf8_ascii_table);

	char buffer[33];


	Symbol symbols[] = { {"a", 5},
						 {"b", 2},
						 {"c", 1},
						 {"d", 1},
						 {"r", 2}};
			 

	HuffmanTree* tree = create_tree(symbols, 5); 

	show_tree(tree->root, 1);

	qsort(symbols, 5, sizeof(Symbol), compare_symbols);

	set_codes(tree, symbols, 5);

	printf("Symbols: \n");
	for(int i = 0; i < 5; i++){
		get_bin_str(&symbols[i], buffer);
		printf("\tSymbol: %s, code: %s (%ld), length: %d\n",symbols[i].repr, buffer, symbols[i].code.value, symbols[i].code.length);
	}
	
	
	destroy_tree(tree);



	return 0;
}
