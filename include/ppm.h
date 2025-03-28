#ifndef __PPM_H__
#define __PPM_H__
#include "huffman_tree.h"
#include "hash_map.h"
#include <stdlib.h>
#include "utils.h"

#define TABLE_SIZE 50000

#define RHO "`"

enum{
    EQPROB_TABLE,
    K0_TABLE,
    K1_TABLE,
    K2_TABLE,
    K3_TABLE,
    K4_TABLE,
    K5_TABLE,
};

/*a-z + space*/
#define ALPHABET_SIZE 27

void initialize_equiprob_table(ContextInfo* ctx);
void initialize_ppm_table(ContextInfo* ctx);
void compress(char* input_filepath, char* output_filepath);
void write_code_to_file(FILE* outfile, Symbol* sb, uint8_t* outbuffer, int* remaining_bits);



#endif