#include "ppm.h"

void initialize_equiprob_table(ContextInfo *ctx)
{
    /*A tabela k=-1 inicialmente deve conter todos os símbolos do alfabeto, com contadores iguais a 1*/
    ctx->n_symb = 27;
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
    set_codes(ctx->tree, all_symbols, 27);

    printf("K=-1 - Symbols:\n");
    char code_str[30];
    for(int i = 0; i < ctx->n_symb; i++){
        get_bin_str(all_symbols[i], code_str);
        printf("Symbol: %s, code: %s\n", all_symbols[i]->repr, code_str);
    }

    free(all_symbols);
}

void initialize_ppm_table(ContextInfo *ctx)
{
    ctx->n_symb = 0;
    ctx->symb_table = (Item **)malloc(sizeof(Item *) * TABLE_SIZE);
    ctx->tree = NULL;

    for (int i = 0; i < TABLE_SIZE; i++)
        ctx->symb_table[i] = NULL;
}

void write_code_to_file(FILE *outfile, Symbol *sb, uint8_t *outbuffer, int *remaining_bits)
{
    if (*remaining_bits >= sb->code.length)
    { /*Se for possível inserir o código inteiro no byte atual*/
        *outbuffer = *outbuffer << sb->code.length;
        *outbuffer = *outbuffer | sb->code.value;
        *remaining_bits -= sb->code.length;
    }
    else
    { /*Se não for possível...*/
        *outbuffer = *outbuffer << *remaining_bits;
        /*Número de bits restantes no código (que precisam ser inseridos no próximo byte)*/
        int n_rest = sb->code.length - *remaining_bits;

        uint64_t code_slice = sb->code.value >> n_rest; /*Extraindo o pedaço do código que cabe no byte atual*/
        *outbuffer = *outbuffer | code_slice;
        /*Escrevendo na saída*/
        fputc(*outbuffer, outfile);
        *outbuffer = 0; /*Limpando o buffer*/
        /*Extraindo somente os bits que restaram da inserção anterior*/
        int mask = pow(2, n_rest) - 1;
        code_slice = sb->code.value & mask;
        *outbuffer = *outbuffer | code_slice;
        *remaining_bits = 8 - n_rest;
    }
}

void rebuild_tree(ContextInfo* ctx){

    if(ctx->n_symb){
        Symbol** extracted_symbols = NULL;
        /*Reconstruindo árvore de contexto k=0*/
        destroy_tree(ctx->tree);
        extracted_symbols = extract_symbols(ctx, TABLE_SIZE);
        ctx->tree = create_tree(extracted_symbols, ctx->n_symb);
        set_codes(ctx->tree, extracted_symbols, ctx->n_symb);

        char code_str[30];
        printf("<<< all symbols >>>\n");
        for(int i = 0; i < ctx->n_symb; i++){
            get_bin_str(extracted_symbols[i], code_str);
            printf("Symbol: %s, code: %s, counter: %d\n", extracted_symbols[i]->repr, code_str, extracted_symbols[i]->counter);
        }
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
    uint8_t file_length_portion = file_size & (255 << 24);
    fputc(file_length_portion, outfile);
    file_length_portion = file_size & (255 << 16);
    fputc(file_length_portion, outfile);
    file_length_portion = file_size & (255 << 8);
    fputc(file_length_portion, outfile);
    file_length_portion = file_size & 255;
    fputc(file_length_portion, outfile);

    fseek(file, 0L, SEEK_SET);

    int n_bits = 0;

    uint8_t outbuffer = 0;  /*Byte a ser mandado para a saída.*/
    int remaining_bits = 8; /*Número restante de bits livres em outbuffer*/

    while ((c = fgetc(file)) != EOF)
    {
        sprintf(buffer, "%c", c);
        sb = get_item(contexts[K0_TABLE].symb_table, buffer);
        if (sb != NULL)
        { /** Se o símbolo atual FOI encontrado na tabela k=0 */
            /*Icrementa o contador do símbolo*/
            increment_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, buffer);

            get_bin_str(sb, code_str);
            show_tree(contexts[K0_TABLE].tree->root, 1);
            printf("Símbolo '%s' encontrado na tabela k=0...\033[0;32mcom código %s\033[0m\n", sb->repr, code_str);
            /*Manda o código do símbolo para a saída*/
            write_code_to_file(outfile, sb, &outbuffer, &remaining_bits);
            n_bits += sb->code.length;
            /*Reconstruindo a árvore de k=0*/
            rebuild_tree(&contexts[K0_TABLE]);
        }
        else
        { /*Se o símbolo atual NÃO FOI encontrado na tabela k=0*/
            printf("Símbolo '%c' não encontrado na tabela k=0, chaveando para k=-1...\n", c);
            sb = get_item(contexts[EQPROB_TABLE].symb_table, buffer);
            if (sb != NULL)
            {
                printf("Símbolo %s encontrado na tabela k=-1 (N = %d)\n", buffer, contexts[EQPROB_TABLE].n_symb);

                /*Adicionando o símbolo na tabela k=0 */
                add_to_context(&contexts[K0_TABLE], buffer);
                Symbol *rho = get_item(contexts[K0_TABLE].symb_table, RHO);
                /*Adicionando 'rho' na tabela k=0 (ou incrementando)*/
                if (rho != NULL)
                { /**Se o símbolo 'rho' já existe na tabela k=0 */
                    /*Mandando o código de 'rho' para a saída*/
                    increment_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, RHO);
                    write_code_to_file(outfile, rho, &outbuffer, &remaining_bits);
                    n_bits += rho->code.length;
                    get_bin_str(rho, code_str);
                    printf("Árvore k=0\n");
                    show_tree(contexts[K0_TABLE].tree->root, 1);
                    printf("\t\tMandando código de rho (%s) para a saída. \033[0;32mCódigo: %s, length: %d\033[0m\n", rho->repr, code_str, rho->code.length);
                }
                else
                { /*Se o símbolo 'rho' ainda não estiver na tabela k=0*/
                    /*Adiciona 'rho' à tabela*/
                   add_to_context(&contexts[K0_TABLE], RHO);
                }

                /*Mandando código do símbolo para a saída (a partir da tabela de equiprobabilidade)*/
                if (contexts[EQPROB_TABLE].n_symb > 1)
                {
                    get_bin_str(sb, code_str);
                    printf("\t\tMandando código de %s para a saída. \033[0;32mCódigo: %s\033[0m\n", sb->repr, code_str);
                    write_code_to_file(outfile, sb, &outbuffer, &remaining_bits);
                    n_bits += sb->code.length;
                }
                else
                {
                    /*Se não houverem mais símbolos na tabela de equiprobabilidade, então
                      não há mais símbolos desconhecidos e podemos remover o rho da tabela.
                    */
                    remove_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, RHO);
                    contexts[K0_TABLE].n_symb--;
                }

                /*Removendo símbolo da tabela k=-1*/
                remove_item(contexts[EQPROB_TABLE].symb_table, TABLE_SIZE, buffer);
                contexts[EQPROB_TABLE].n_symb--;
                /*Reconstruindo árvores*/

                /*Reconstruindo a árvore da tabela de equiprobabilidade*/
                rebuild_tree(&contexts[EQPROB_TABLE]);

                
                // Reconstruir árvore k = 0 
                rebuild_tree(&contexts[K0_TABLE]);
                show_tree(contexts[K0_TABLE].tree->root, 1);

                

            }
        }
    }

    if (remaining_bits)
    {
        outbuffer = outbuffer << remaining_bits;
        fputc(outbuffer, outfile);
    }

    printf("Number of bits: %d\n", n_bits);
    // printf("Saída: %s\n", outputstring);

    fclose(file);
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
    file_size = byte & (255 << 24);
    byte = fgetc(file);
    file_size = byte & (255 << 16);
    byte = fgetc(file);
    file_size = byte & (255 << 8);
    byte = fgetc(file);
    file_size = byte & 255;

    int explored = 0;

    // printf("Size of file: %d\n", file_size);

    Code *code = (Code *)malloc(sizeof(Code));
    int remaining_bits = 8;
    code->value = 0;
    code->length = 0;
    Symbol *found_symbol = NULL;

    bool skip_k0 = false;
    while (explored < file_size)
    {

        byte = fgetc(file);

        printf("Lendo byte: %d\n", byte);
        remaining_bits = 8;

        while(remaining_bits){
            code->value = code->value << 1;
            code->value |= (byte & (int)pow(2, remaining_bits) - 1) >> (remaining_bits - 1);
            code->length++;
            remaining_bits--;

            char code_str[30];

            Symbol xs;
            xs.code = *code;
            get_bin_str(&xs, code_str);

            printf("Code: %s\n", code_str);

            printf("skip_k0 = %d\n", skip_k0);

            /*SE SKIP_K0 == False && o símbolo estiver em k = 0*/
                /*SE for um rho*/
                    /*Reseta "code", faz SKIP_K0=True e continua para a próxima iteração*/
                /*SENÃO*/
                    /*Manda o símbolo para a saída*/
                    /*Incrementa o contador do símbolo em k=0*/
                    /*Remove o símbolo de k=-1*/
            /*SENÃO, SE o símbolo estiver em k=-1*/
                /*Manda o símbolo para a saída*/
                /*Faz SKIP_K0=False*/

            /*Reconstrói árvore k=0*/
            /*Reconstrói árvore k=-1*/


            if(!skip_k0 && (found_symbol = search_code(contexts[K0_TABLE].symb_table, code)) != NULL){
                if(!strcmp(found_symbol->repr, RHO)){
                    code->length = code->value = 0;
                    skip_k0 = true;
                    printf("\033[0;35mEncontrou um rho!\033[0m\n");
                    getchar();
                }else{
                    printf("Mandando \033[0;31m\"%s\"\033[0m\n", found_symbol->repr);
                    increment_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, found_symbol->repr);
                    code->length = code->value = 0;
                    getchar();
                }
            }else if((found_symbol = search_code(contexts[EQPROB_TABLE].symb_table, code)) != NULL){
                printf("Mandando \033[0;32m\"%s\"\033[0m\n", found_symbol->repr);
                add_to_context(&contexts[K0_TABLE], found_symbol->repr);

                if(get_item(contexts[K0_TABLE].symb_table, RHO) != NULL){
                    increment_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, RHO);
                }else{
                    add_to_context(&contexts[K0_TABLE], RHO);
                }
                remove_item(contexts[EQPROB_TABLE].symb_table, TABLE_SIZE, found_symbol->repr);
                contexts[EQPROB_TABLE].n_symb--;
                skip_k0 = false;
                code->value = code->length = 0;
                getchar();
            }

            rebuild_tree(&contexts[EQPROB_TABLE]);
            rebuild_tree(&contexts[K0_TABLE]);

        }
    }

    fclose(file);
    fclose(outfile);
}