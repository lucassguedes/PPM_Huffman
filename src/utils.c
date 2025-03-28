#include "utils.h"

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