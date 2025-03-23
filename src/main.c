#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"

char utf8_ascii_table[256][256];

int main(int argc, char** argv){

	
	if(argc < 3){
		printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
		return -1;
	}
	
	initialize_convertion_table(utf8_ascii_table);
	
	format_file(argv[1], argv[2], utf8_ascii_table);


	return 0;
}
