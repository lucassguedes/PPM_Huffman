#ifndef __PPM_H__
#define __PPM_H__
#include "huffman_tree.h"
#include "hash_map.h"
#include <stdlib.h>
#include "utils.h"

#define TABLE_SIZE 50000
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

void initialize_equiprob_table(ContextInfo* ctx);
void initialize_ppm_table(ContextInfo* ctx);
void compress(char* input_filepath, char* output_filepath, int K);
void decompress(char* input_filepath, char* output_filepath, int K);
void write_code_to_file(FILE* outfile, Symbol* sb, uint8_t* outbuffer, int* remaining_bits);



#endif