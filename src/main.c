#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"
#include "hash_map.h"

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

	const int hash_map_size = 50000;

	Item* map[hash_map_size];

	for(int i = 0; i < hash_map_size; i++) map[i] = NULL;

	Symbol s1;

	s1.counter = 5;
	s1.repr = "a";

	add_item(map, s1);

	s1.counter = 2;
	s1.repr = "b";

	add_item(map, s1);


	s1.counter = 2;
	s1.repr = "r";

	add_item(map, s1);


	s1.counter = 1;
	s1.repr = "c";

	add_item(map, s1);


	s1.counter = 1;
	s1.repr = "d";

	add_item(map, s1);


	show_map(map, hash_map_size);

	Symbol* s2 = get_item(map, "c");

	printf("Symbol %s, counter: %d\n", s2->repr, s2->counter);

	destroy_map(map, hash_map_size);








	return 0;
}
