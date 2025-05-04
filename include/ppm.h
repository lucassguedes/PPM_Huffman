#ifndef __PPM_H__
#define __PPM_H__
#include "huffman_tree.h"
#include "context.h"
#include <stdlib.h>
#include "utils.h"

#define EMPTY_CONTEXT "---"

#define RHO "`"

enum{
    K1,
    K2,
    K3,
    K4,
    K5,
};

/*a-z + space*/
#define ALPHABET_SIZE 27

void initialize_equiprob_table(ContextInfo* ctx, bool load_model);
void initialize_ppm_table(ContextInfo* ctx);
void compress(char* input_filepath, char* output_filepath, bool save_model, char* path_to_save_model, bool load_model, char* loaded_model_path);
void decompress(char* input_filepath, char* output_filepath);
void write_code_to_file(FILE* outfile, Symbol* sb, uint8_t* outbuffer, int* remaining_bits);



#endif