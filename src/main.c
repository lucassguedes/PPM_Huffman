#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"

char utf8_ascii_table[256][256];

void show_tree(Node* root, int ntab){
	if(root == NULL){
		return;
	}

	for(int i = 0; i < ntab; i++)printf("\t");
	printf("Node - counter: %d\n", root->counter);
	for(int i = 0; i < ntab + 1; i++)printf("\t");
	printf("Children:\n");
	show_tree(root->left_child, ntab + 2);
	show_tree(root->right_child, ntab + 2);
	return;
}

int main(int argc, char** argv){

	
	// if(argc < 3){
	// 	printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
	// 	return -1;
	// }
	
	// initialize_convertion_table(utf8_ascii_table);
	
	// format_file(argv[1], argv[2], utf8_ascii_table);

	int counters[] = {4, 6, 2, 5, 7, 9, 8, 1, 3};

	Node* root = create_tree(counters, 9);

	show_tree(root, 1);




	return 0;
}
