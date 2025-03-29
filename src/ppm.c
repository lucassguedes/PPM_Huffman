#include "ppm.h"

void initialize_equiprob_table(ContextInfo *ctx)
{
    /*A tabela k=-1 inicialmente deve conter todos os símbolos do alfabeto, com contadores iguais a 1*/
    ctx->n_symb = 27;
    ctx->max_search_length = 0;
    ctx->symb_table = (Item **)malloc(sizeof(Item *) * TABLE_SIZE);
    ctx->tree = NULL;

    for (int i = 0; i < TABLE_SIZE; i++)
        ctx->symb_table[i] = NULL;

    /*Adicionando símbolos do alfabeto*/
    Symbol s;
    s.repr = (char *)malloc(sizeof(char) * 2);
    strcpy(s.repr, " ");
    s.counter = 1;

    add_item(ctx->symb_table, s);

    strcpy(s.repr, "a");
    for (int i = 0; i < 26; i++)
    {
        add_item(ctx->symb_table, s);
        s.repr[0]++;
    }

    free(s.repr);

    /*Extraindo símbolos da tabela de símbolos - esta operação é feita sempre que a árvore
      precisa ser reconstruída.
    */
    Symbol **all_symbols = extract_symbols(ctx, TABLE_SIZE);
    ctx->tree = create_tree(all_symbols, 27);
    /*Atualiza os códigos dos símbolos de acordo com a estrutura da árvore*/
    set_codes(ctx);

    // printf("K=-1 - Symbols:\n");
    // char code_str[30];
    // for(int i = 0; i < ctx->n_symb; i++){
    //     get_bin_str(all_symbols[i], code_str);
    //     printf("Symbol: %s, code: %s\n", all_symbols[i]->repr, code_str);
    // }

    free(all_symbols);
}

void initialize_ppm_table(ContextInfo *ctx)
{
    ctx->n_symb = 0;
    ctx->max_search_length = 0;
    ctx->symb_table = (Item **)malloc(sizeof(Item *) * TABLE_SIZE);
    ctx->tree = NULL;

    for (int i = 0; i < TABLE_SIZE; i++)
        ctx->symb_table[i] = NULL;
}

void write_code_to_file(FILE *outfile, Symbol *sb, uint8_t *outbuffer, int *outbuffer_length)
{
    

    int bits_to_write = sb->code.length;
    uint64_t code = sb->code.value;

    int bits_to_ignore;
    while(bits_to_write > *outbuffer_length){
        bits_to_ignore = bits_to_write - *outbuffer_length; //bits_to_ignore -> número de bits que serão deixados para o próximo byte

        // printf("\033[0;31mPrecisamos de %d bits, mas só temos %d bits...\n", bits_to_write, *outbuffer_length);
        
        *outbuffer = *outbuffer << *outbuffer_length; /*Liberando espaço para inserir um pedaço do código*/
        
        uint64_t code_slice = code >> bits_to_ignore; /*Extraindo o pedaço do código que cabe no byte atual*/
        *outbuffer = *outbuffer | code_slice;
        
        // printf("\t\033[0;31mNeste byte, vamos colocar apenas %d bits mais significativos do código, que terá valor %d\033[0m\n",*outbuffer_length, code_slice);

        code = code & ((int)pow(2, bits_to_ignore) - 1); /*Removendo a parte do código que está para ser escrita*/
        bits_to_write -= *outbuffer_length; 

        
        /*Escrevendo na saída*/        
        fputc(*outbuffer, outfile);
        *outbuffer = 0; /*Limpando o buffer*/
        *outbuffer_length = 8;
        // printf("\t\033[0;31mAgora restam %d bits, e o código restante é %d\033[0m\n",bits_to_write, code);
    }

    // printf("\033[0;32mFinalmente temos espaço o suficiente! Escreveremos %d bits restantes, com valor %d\033[0m\n",bits_to_write,code);
    *outbuffer = *outbuffer << bits_to_write;
    *outbuffer = *outbuffer | code;
    *outbuffer_length -= bits_to_write;

    if(*outbuffer_length == 0){
        fputc(*outbuffer, outfile);
        *outbuffer = 0;
        *outbuffer_length = 8;
    }
}

void rebuild_tree(ContextInfo* ctx){

    if(ctx->n_symb){
        Symbol** extracted_symbols = NULL;
        /*Reconstruindo árvore de contexto k=0*/
        destroy_tree(ctx->tree);
        extracted_symbols = extract_symbols(ctx, TABLE_SIZE);
        ctx->tree = create_tree(extracted_symbols, ctx->n_symb);
        set_codes(ctx);

        // char code_str[30];
        // printf("<<< all symbols >>>\n");
        // for(int i = 0; i < ctx->n_symb; i++){
        //     get_bin_str(extracted_symbols[i], code_str);
        //     printf("Symbol: %s, code: %s, counter: %d\n", extracted_symbols[i]->repr, code_str, extracted_symbols[i]->counter);
        // }
        free(extracted_symbols);
        extracted_symbols = NULL;
        return;
    }

    ctx->tree = NULL;
}

void add_to_context(ContextInfo* ctx, char* repr){
    Symbol new_symbol;
    new_symbol.repr = (char *)malloc(sizeof(char) * (strlen(repr) + 1));
    new_symbol.counter = 1;
    new_symbol.code.length = new_symbol.code.value = 0;
    strcpy(new_symbol.repr, repr);
    add_item(ctx->symb_table, new_symbol);
    ctx->n_symb++;
    free(new_symbol.repr);
}

void compress(char *input_filepath, char *output_filepath)
{

    /*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
    ContextInfo contexts[2];

    initialize_equiprob_table(&contexts[EQPROB_TABLE]);
    initialize_ppm_table(&contexts[K0_TABLE]);

    FILE *file = fopen(input_filepath, "r");
    FILE *outfile = fopen(output_filepath, "w");

    char c;
    char buffer[5];
    Symbol *sb;

    char code_str[30];

    fseek(file, 0L, SEEK_END);

    // Calculando o tamanho do arquivo (em bytes)
    uint32_t file_size = ftell(file);
    printf("Size of file: %d\n", file_size);

    /*Os primeiros 4 bytes do arquivo comprimido indicam a quantidade de bytes do arquivo original*/
    uint8_t file_length_portion = (file_size & (255 << 24)) >> 24;
    fputc(file_length_portion, outfile);
    file_length_portion = (file_size & (255 << 16)) >> 16;
    fputc(file_length_portion, outfile);
    file_length_portion = (file_size & (255 << 8)) >> 8;
    fputc(file_length_portion, outfile);
    file_length_portion = file_size & 255;
    fputc(file_length_portion, outfile);

    fseek(file, 0L, SEEK_SET);

    int n_bits = 0;

    uint8_t outbuffer = 0;  /*Byte a ser mandado para a saída.*/
    int outbuffer_length = 8; /*Número restante de bits livres em outbuffer*/
    char word[30];
    sprintf(word, "");

    int explored = 0;
    while ((c = fgetc(file)) != EOF)
    {
        // printf("Progress: %0.2f%%\n", (explored / (double)file_size) * 100);
        sprintf(buffer, "%c", c);
        sb = get_item(contexts[K0_TABLE].symb_table, buffer);
        if (sb != NULL)
        { /** Se o símbolo atual FOI encontrado na tabela k=0 */
            /*Icrementa o contador do símbolo*/
            increment_item(contexts[K0_TABLE].symb_table, buffer);

            // get_bin_str(sb, code_str);
            // show_tree(contexts[K0_TABLE].tree->root, 1);
            // printf("Símbolo '%s' encontrado na tabela k=0...\033[0;32mcom código %s\033[0m\n", sb->repr, code_str);

            // if(!strcmp(sb->repr, " ")){
            //     printf("\033[0;31mPalavra: %s\033[0m\n", word);
            //     if(!strcmp(word, "juvenil")){
            //         getchar();
            //     }
            //     sprintf(word, "");

            // }else{
            //     strcat(word, sb->repr);
            // }

            /*Manda o código do símbolo para a saída*/
            write_code_to_file(outfile, sb, &outbuffer, &outbuffer_length);
            n_bits += sb->code.length;
            /*Reconstruindo a árvore de k=0*/
            rebuild_tree(&contexts[K0_TABLE]);
            explored++;
        }
        else
        { /*Se o símbolo atual NÃO FOI encontrado na tabela k=0*/
            // printf("Símbolo '%c' não encontrado na tabela k=0, chaveando para k=-1...\n", c);
            sb = get_item(contexts[EQPROB_TABLE].symb_table, buffer);
            if (sb != NULL)
            {
                explored++;
                // printf("Símbolo %s encontrado na tabela k=-1 (N = %d)\n", buffer, contexts[EQPROB_TABLE].n_symb);

                /*Adicionando o símbolo na tabela k=0 */
                add_to_context(&contexts[K0_TABLE], buffer);
                Symbol *rho = get_item(contexts[K0_TABLE].symb_table, RHO);
                /*Adicionando 'rho' na tabela k=0 (ou incrementando)*/
                if (rho != NULL)
                { /**Se o símbolo 'rho' já existe na tabela k=0 */
                    /*Mandando o código de 'rho' para a saída*/
                    increment_item(contexts[K0_TABLE].symb_table, RHO);
                    write_code_to_file(outfile, rho, &outbuffer, &outbuffer_length);
                    n_bits += rho->code.length;
                    // get_bin_str(rho, code_str);
                    // printf("Árvore k=0\n");
                    // show_tree(contexts[K0_TABLE].tree->root, 1);
                    // printf("\t\tMandando código de rho (%s) para a saída. \033[0;32mCódigo: %s, length: %d\033[0m\n", rho->repr, code_str, rho->code.length);
                }
                else
                { /*Se o símbolo 'rho' ainda não estiver na tabela k=0*/
                    /*Adiciona 'rho' à tabela*/
                   add_to_context(&contexts[K0_TABLE], RHO);
                }

                /*Mandando código do símbolo para a saída (a partir da tabela de equiprobabilidade)*/
                if (contexts[EQPROB_TABLE].n_symb > 1)
                {
                    // get_bin_str(sb, code_str);
                    // printf("\t\tMandando código de %s para a saída. \033[0;32mCódigo: %s\033[0m\n", sb->repr, code_str);
                    // if(!strcmp(sb->repr, " ")){
                    //     printf("\033[0;31mPalavra: %s\033[0m\n", word);
                    //     sprintf(word, "");
                    //     if(!strcmp(word, "juvenil")){
                    //         getchar();
                    //     }
                    // }else{
                    //     strcat(word, sb->repr);
                    // }
                    write_code_to_file(outfile, sb, &outbuffer, &outbuffer_length);
                    n_bits += sb->code.length;
                }
                else
                {
                    /*Se não houverem mais símbolos na tabela de equiprobabilidade, então
                      não há mais símbolos desconhecidos e podemos remover o rho da tabela.
                    */
                    remove_item(contexts[K0_TABLE].symb_table, RHO);
                    contexts[K0_TABLE].n_symb--;
                }

                /*Removendo símbolo da tabela k=-1*/
                remove_item(contexts[EQPROB_TABLE].symb_table, buffer);
                contexts[EQPROB_TABLE].n_symb--;
                /*Reconstruindo árvores*/

                /*Reconstruindo a árvore da tabela de equiprobabilidade*/
                rebuild_tree(&contexts[EQPROB_TABLE]);

                
                // Reconstruir árvore k = 0 
                rebuild_tree(&contexts[K0_TABLE]);
                // show_tree(contexts[K0_TABLE].tree->root, 1);

                

            }
        }
    }

    if (outbuffer_length > 0 && outbuffer_length < 8)
    {
        printf("O outbuffer ainda tem %d bits livres!\n", outbuffer_length);
        outbuffer = outbuffer << outbuffer_length;
        fputc(outbuffer, outfile);
    }

    printf("Number of bits: %d\n", n_bits);
    // printf("Saída: %s\n", outputstring);

    fclose(file);
    fclose(outfile);
    destroy_tree(contexts[EQPROB_TABLE].tree);
    destroy_tree(contexts[K0_TABLE].tree);

    destroy_map(contexts[EQPROB_TABLE].symb_table, TABLE_SIZE);
    destroy_map(contexts[K0_TABLE].symb_table, TABLE_SIZE);
}

Symbol *search_code(Item *map[], Code *code)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Item *it = map[i];

        char aux_str[30];

        while (it != NULL)
        {
            get_bin_str(it->value, aux_str);
            if (it->value->code.value == code->value && it->value->code.length == code->length)
            {
                return it->value;
            }
            it = it->next;
        }
    }
    return NULL;
}

void decompress(char *input_filepath, char *output_filepath)
{
    /*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
    ContextInfo contexts[2];

    initialize_equiprob_table(&contexts[EQPROB_TABLE]);
    initialize_ppm_table(&contexts[K0_TABLE]);

    FILE *file = fopen(input_filepath, "r");
    FILE *outfile = fopen(output_filepath, "w");

    uint8_t byte;

    uint32_t file_size = 0;

    byte = fgetc(file);
    file_size += byte << 24;
    byte = fgetc(file);
    file_size += byte << 16;
    byte = fgetc(file);
    file_size += byte << 8;
    byte = fgetc(file);
    file_size += byte;

    int explored = 0;

    printf("Size of file: %d\n", file_size);

    Code *code = (Code *)malloc(sizeof(Code));
    int bits_to_read = 8;
    code->value = 0;
    code->length = 0;
    Symbol *found_symbol = NULL;

    bool skip_k0 = false;
    char word[30];

    sprintf(word, "");
    while (explored < file_size)
    {

        byte = fgetc(file);

        // printf("Lendo byte: %d\n", byte);
        bits_to_read = 8;

        while(bits_to_read){
            if(explored == file_size){
                break;
            }

            // printf("explored: %d/%d\n", explored, file_size);
            code->value = code->value << 1;
            code->value |= (byte & (int)pow(2, bits_to_read) - 1) >> (bits_to_read - 1);
            code->length++;
            bits_to_read--;

            // printf("Code: %s, max_search_length(k=0): %d, max_search_length(k=-1): %d\n", code_str, contexts[K0_TABLE].max_search_length, contexts[EQPROB_TABLE].max_search_length);
            // printf("skip_k0 = %d\n", skip_k0);

            // printf("\033[0;35mNUMERO DE SÍMBOLOS EM K=-1: %d\033[0m\n", contexts[EQPROB_TABLE].n_symb);


            if(!skip_k0){
                if((found_symbol = search_code(contexts[K0_TABLE].symb_table, code)) != NULL){
                    if(!strcmp(found_symbol->repr, RHO)){
                        code->length = code->value = 0;
                        skip_k0 = true;
                        // printf("\033[0;35mEncontrou um rho! N(k=0) = %d\033[0m\n", contexts[EQPROB_TABLE].n_symb);

                        if(contexts[EQPROB_TABLE].n_symb == 1){
                            explored++;
                            Symbol** extracted_symbols = extract_symbols(&contexts[EQPROB_TABLE], TABLE_SIZE);                            
                            fprintf(outfile, "%s", extracted_symbols[0]->repr);

                            // if(!strcmp(extracted_symbols[0]->repr, " ")){
                            //     printf("\033[0;31mPalavra: %s\033[0m\n", word);
                            //     getchar();
                            //     sprintf(word, "");
                            // }else{
                            //     strcat(word, extracted_symbols[0]->repr);
                            // }

                            add_to_context(&contexts[K0_TABLE], extracted_symbols[0]->repr);

                            remove_item(contexts[EQPROB_TABLE].symb_table, extracted_symbols[0]->repr);
                            contexts[EQPROB_TABLE].n_symb--;

                            remove_item(contexts[K0_TABLE].symb_table, RHO);
                            contexts[K0_TABLE].n_symb--;
                            rebuild_tree(&contexts[EQPROB_TABLE]);
                            rebuild_tree(&contexts[K0_TABLE]);
                            skip_k0 = false;
                        }
                    }else{
                        // printf("Mandando \033[0;31m\"%s\"\033[0m\n", found_symbol->repr);
                        fprintf(outfile, "%s", found_symbol->repr);
                        increment_item(contexts[K0_TABLE].symb_table, found_symbol->repr);
                        rebuild_tree(&contexts[K0_TABLE]);
                        code->length = code->value = 0;

                        // if(!strcmp(found_symbol->repr, " ")){
                        //     printf("\033[0;31mPalavra: %s\033[0m\n", word);
                        //     getchar();
                        //     sprintf(word, "");
                        // }else{
                        //     strcat(word, found_symbol->repr);
                        // }

                        explored++;
                    }
                    continue;
                }else if(code->length < contexts[K0_TABLE].max_search_length){
                    continue;
                }
            }

            
            if((found_symbol = search_code(contexts[EQPROB_TABLE].symb_table, code)) != NULL){
                explored++;
                // printf("Mandando \033[0;32m\"%s\"\033[0m\n", found_symbol->repr);
                // if(!strcmp(found_symbol->repr, " ")){
                //     printf("\033[0;31mPalavra: %s\033[0m\n", word);
                //     sprintf(word, "");
                //     getchar();
                // }else{
                //     strcat(word, found_symbol->repr);
                // }
                fprintf(outfile, "%s", found_symbol->repr);
                add_to_context(&contexts[K0_TABLE], found_symbol->repr);

                if(get_item(contexts[K0_TABLE].symb_table, RHO) != NULL){
                    increment_item(contexts[K0_TABLE].symb_table, RHO);
                }else{
                    add_to_context(&contexts[K0_TABLE], RHO);
                }
                remove_item(contexts[EQPROB_TABLE].symb_table, found_symbol->repr);
                contexts[EQPROB_TABLE].n_symb--;

                skip_k0 = false;
                code->value = code->length = 0;
            }

            rebuild_tree(&contexts[EQPROB_TABLE]);
            rebuild_tree(&contexts[K0_TABLE]);

        }
    }

    fclose(file);
    fclose(outfile);

    destroy_tree(contexts[EQPROB_TABLE].tree);
    destroy_tree(contexts[K0_TABLE].tree);

    destroy_map(contexts[EQPROB_TABLE].symb_table, TABLE_SIZE);
    destroy_map(contexts[K0_TABLE].symb_table, TABLE_SIZE);
}