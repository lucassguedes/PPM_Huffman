#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"
#include "hash.h"

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

	printf("hash(%s) = %d\n", "a", hash("a"));
	printf("hash(%s) = %d\n", "a ", hash("a "));
	printf("hash(%s) = %d\n", "b", hash("b"));
	printf("hash(%s) = %d\n", "c", hash("c"));
	printf("hash(%s) = %d\n", "d", hash("d"));
	printf("hash(%s) = %d\n", "r", hash("r"));
	printf("hash(%s) = %d\n", "ab", hash("ab"));
	printf("hash(%s) = %d\n", "ac", hash("ac"));
	printf("hash(%s) = %d\n", "ad", hash("ad"));
	printf("hash(%s) = %d\n", "br", hash("br"));
	printf("hash(%s) = %d\n", "ca", hash("ca"));
	printf("hash(%s) = %d\n", "ra", hash("ra"));








	return 0;
}
