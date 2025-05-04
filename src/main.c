#include <stdio.h>
#include <stdlib.h>
#include "file_formatter.h"
#include "huffman_tree.h"
#include "context.h"
#include "utils.h"
#include "ppm.h"

char utf8_ascii_table[256][256];

int main(int argc, char** argv){

	
	if(argc < 4){
		printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
		return -1;
	}
	

	if(!strcmp(argv[3], "--format")){
		printf("Formatting...\n");
		initialize_convertion_table(utf8_ascii_table);
		format_file(argv[1], argv[2], utf8_ascii_table);
		return 0;
	}


	if(!strcmp(argv[3], "--compress")){
		bool load_model;
		bool save_model;
		char* loaded_model_path = NULL;
		char* path_to_save = NULL;

		if(argc > 4){
			for(int i = 4; i < argc; i++){
				if(!strcmp(argv[i], "--save-model")){
					save_model = true;
					i++;
					path_to_save = (char*)malloc(sizeof(char)*(strlen(argv[i]) + 1));
					strcpy(path_to_save, argv[i]);
					continue;
				}

				if(!strcmp(argv[i], "--load-model")){
					load_model = true;
					i++;
					loaded_model_path = (char*)malloc(sizeof(char)*(strlen(argv[i]) + 1));
					strcpy(loaded_model_path, argv[i]);
					continue;
				}
			}
		}

		compress(argv[1], argv[2], save_model, path_to_save, load_model, loaded_model_path);
		return 0;
	}

	if(!strcmp(argv[3], "--decompress")){
		printf("Decompress...\n");
		printf("input: %s, output: %s\n", argv[1], argv[2]);
		decompress(argv[1], argv[2]);
		return 0;
	}





	return 0;
}
