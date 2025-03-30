#include "ppm.h"

void initialize_equiprob_table(ContextInfo *ctx)
{
    ctx->name = NULL;
    ctx->next = NULL;
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
    ctx->name = NULL;
    ctx->next = NULL;
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

void update_context_str(char context_str[], int k, char* buffer){
    const int str_size = strlen(context_str);
    
    if(!strcmp(context_str, EMPTY_CONTEXT)){
        sprintf(context_str, "%c", buffer[0]);
        return;
    }

    if(str_size == k){
        for(int i = 0; i < str_size - 1; i++){
            context_str[i] = context_str[i+1];
        }
        context_str[str_size-1] = buffer[0];
        context_str[str_size] = '\0';
        return;
    }

    context_str[str_size] = buffer[0];
    context_str[str_size+1] = '\0';
    
}

ContextInfo* get_context(ContextInfo** contexts, char* key){
    const int index = hash(key);

    ContextInfo* ctx = contexts[index];

    while(ctx != NULL){
        if(!strcmp(ctx->name, key)){
            return ctx;
        }
    }

    return NULL;
}

ContextInfo* create_context(ContextInfo** contexts, char* key){
    const int index = hash(key);

    ContextInfo* new_context = (ContextInfo*)malloc(sizeof(ContextInfo));

    initialize_ppm_table(new_context);
    
    new_context->name = key;

    if(contexts[index] == NULL){
        contexts[index] = new_context;
        return;
    }

    ContextInfo* tmp = contexts[index];

    while(tmp->next != NULL){
        tmp = tmp->next;
    }

    tmp->next = new_context;

    return new_context;
}


void compress(char *input_filepath, char *output_filepath)
{

    /*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
    ContextInfo k0_info;
    ContextInfo eqprob_info;

    /***
     * Dicionário  que  contém  os  contextos  iguais  a 1.
     * Cada posição representa um contexto e é acessada por 
     * uma  string,  cada  uma contém um mapa de símbolos e 
     * seus respectivos códigos.
    */
    ContextInfo* k1_table[TABLE_SIZE];
    
    for(int i = 0; i < TABLE_SIZE; i++) k1_table[i] = NULL;

    initialize_equiprob_table(&eqprob_info);
    initialize_ppm_table(&k0_info);

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

    char context_str[5][30];

    strcpy(context_str[K1], EMPTY_CONTEXT);
    strcpy(context_str[K2], EMPTY_CONTEXT);
    strcpy(context_str[K3], EMPTY_CONTEXT);
    strcpy(context_str[K4], EMPTY_CONTEXT);
    strcpy(context_str[K5], EMPTY_CONTEXT);



    ContextInfo* ctx;
    while ((c = fgetc(file)) != EOF)
    {
        printf("########################################################\n");
        sprintf(buffer, "%c", c);
        
        printf("Símbolo atual: \033[0;31m\"%s\"\033[0m\n", buffer);
        printf("Explored: %d / %d\n", explored , file_size);
        
        // update_context_str(context_str[K2], K2+1, buffer);
        // update_context_str(context_str[K3], K3+1, buffer);
        // update_context_str(context_str[K4], K4+1, buffer);
        // update_context_str(context_str[K5], K5+1, buffer)
        //K = 1
        if(strcmp(context_str[K1], EMPTY_CONTEXT)){ // SE a string de contexto não estiver vazia
            ctx = get_context(k1_table, context_str[K1]);

            if(ctx != NULL){ /*O contexto existe*/
                printf("O contexto \"%s\" já existe!\n", context_str[K1]);
                sb = get_item(ctx->symb_table, buffer); /*Verificando se o símbolo atual está neste contexto*/
                
                if(sb != NULL) // Encontrou o símbolo dentro do contexto atual!
                {
                    printf("Encontrou o símbolo \033[0;32m\"%s\"\033[0m no contexto \033[0;32m\"%s\"\033[0m - Código: ", buffer, context_str[K1]);
                    get_bin_str(sb, code_str);
                    printf("%s\n", code_str);
                    increment_item(ctx->symb_table, buffer); // Incrementando o contador do símbolo
                    write_code_to_file(outfile, sb, &outbuffer, &outbuffer_length); // Escrevemos o código no arquivo
                    update_context_str(context_str[K1], K1+1, buffer); //Atualizamos a string de contexto
                    rebuild_tree(ctx);
                    explored++;
                    continue;
                }
                /*Caso não tenha encontrado o código neste contexto*/
                Symbol* rho = get_item(ctx->symb_table, RHO); // Obtém o 'rho'
                get_bin_str(rho, code_str);
                printf("Codificando 'rho' para k = 1 - Código: %s\n", code_str);
                write_code_to_file(outfile, rho, &outbuffer, &outbuffer_length); // Escrevemos o código de 'rho' na saída
                increment_item(ctx->symb_table, RHO); // Incrementa o contador de 'rho'
                add_to_context(ctx, buffer); // Adiciona o novo símbolo ao contexto
                rebuild_tree(ctx);
                
            }else{
                ctx = create_context(k1_table, context_str[K1]);
                add_to_context(ctx, buffer);
                add_to_context(ctx, RHO);
                rebuild_tree(ctx);
            }
        }

        update_context_str(context_str[K1], K1+1, buffer);


        // K = 0
        sb = get_item(k0_info.symb_table, buffer);
        if (sb != NULL)
        {   
            /** Se o símbolo atual FOI encontrado na tabela k=0 */
            /*Icrementa o contador do símbolo*/
            increment_item(k0_info.symb_table, buffer);
            get_bin_str(sb, code_str);
            printf("Símbolo '%s' encontrado na tabela k=0...\033[0;32mcom código %s\033[0m\n", sb->repr, code_str);
            /*Manda o código do símbolo para a saída*/
            write_code_to_file(outfile, sb, &outbuffer, &outbuffer_length);
            n_bits += sb->code.length;
            /*Reconstruindo a árvore de k=0*/
            rebuild_tree(&k0_info);
            explored++;
            continue;
        }
        

        // k = -1
        sb = get_item(eqprob_info.symb_table, buffer); //Procurando símbolo na tabela k = -1
        if (sb != NULL) // Se o símbolo estiver na tabela k = -1
        {
            explored++;

            /*Adicionando o símbolo na tabela k=0 */
            add_to_context(&k0_info, buffer);
            Symbol *rho = get_item(k0_info.symb_table, RHO);
            /*Adicionando 'rho' na tabela k=0 (ou incrementando)*/
            if (rho != NULL)
            { /**Se o símbolo 'rho' já existe na tabela k=0 */
                /*Mandando o código de 'rho' para a saída*/
                increment_item(k0_info.symb_table, RHO);
                get_bin_str(rho, code_str);
                printf("Codificando um 'rho' de K=0 na saída - Código: %s\n", code_str);
                write_code_to_file(outfile, rho, &outbuffer, &outbuffer_length);
                n_bits += rho->code.length;
            }
            else
            {   /*Se o símbolo 'rho' ainda não estiver na tabela k=0*/
                /*Adiciona 'rho' à tabela*/
                add_to_context(&k0_info, RHO);
            }

            /*Mandando código do símbolo para a saída (a partir da tabela de equiprobabilidade)*/
            if (eqprob_info.n_symb > 1) 
            {
                get_bin_str(sb, code_str);
                printf("Codificando \"%s\" a partir da tabela k = -1 - Código: %s\033[0m\n", sb->repr, code_str);
                write_code_to_file(outfile, sb, &outbuffer, &outbuffer_length);
                n_bits += sb->code.length;
            }
            else
            {
                /*Se não houverem mais símbolos na tabela de equiprobabilidade, então
                    não há mais símbolos desconhecidos e podemos remover o rho da tabela.
                */
                remove_item(k0_info.symb_table, RHO);
                k0_info.n_symb--;
            }

            /*Removendo símbolo da tabela k=-1*/
            remove_item(eqprob_info.symb_table, buffer);
            eqprob_info.n_symb--;
            /*Reconstruindo árvores*/

            /*Reconstruindo a árvore da tabela de equiprobabilidade*/
            rebuild_tree(&eqprob_info);

            
            // Reconstruir árvore k = 0 
            rebuild_tree(&k0_info);
            // show_tree(k0_info.tree->root, 1);
        }
    }

    if (outbuffer_length > 0 && outbuffer_length < 8)
    {
        outbuffer = outbuffer << outbuffer_length;
        fputc(outbuffer, outfile);
    }


    fclose(file);
    fclose(outfile);
    destroy_tree(eqprob_info.tree);
    destroy_tree(k0_info.tree);

    destroy_map(eqprob_info.symb_table, TABLE_SIZE);
    destroy_map(k0_info.symb_table, TABLE_SIZE);
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
    ContextInfo k0_info;
    ContextInfo eqprob_info;

    ContextInfo* k1_table[TABLE_SIZE]; /*Dicionário que contém os contextos iguais a 1*/

    for(int i = 0; i < TABLE_SIZE; i++) k1_table[i] = NULL;


    initialize_equiprob_table(&eqprob_info);
    initialize_ppm_table(&k0_info);

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
    bool skip_k1 = false;

    char word[30];

    sprintf(word, "");

    char context_str[5][30];

    strcpy(context_str[K1], EMPTY_CONTEXT);
    strcpy(context_str[K2], EMPTY_CONTEXT);
    strcpy(context_str[K3], EMPTY_CONTEXT);
    strcpy(context_str[K4], EMPTY_CONTEXT);
    strcpy(context_str[K5], EMPTY_CONTEXT);
    ContextInfo* ctx = NULL;

    char code_str[30];
    while (explored < file_size)
    {

        byte = fgetc(file);

        // printf("Lendo byte: %d\n", byte);
        bits_to_read = 8;

        while(bits_to_read){
            if(explored == file_size){
                break;
            }

            code->value = code->value << 1;
            code->value |= (byte & (int)pow(2, bits_to_read) - 1) >> (bits_to_read - 1);
            code->length++;
            bits_to_read--;

            Symbol xs;

            xs.code = *code;

            get_bin_str(&xs, code_str);
            printf("\033[0;32mCode: %s\033[0m\n", code_str);


            // K = 1
            if(!skip_k1 && strcmp(context_str[K1], EMPTY_CONTEXT)){ // Se a string de contexto não for vazia
                /* Verificando a existência do contexto*/
                ctx = get_context(k1_table, context_str[K1]); 
                found_symbol = NULL;
                bool is_rho = false;

                if(ctx != NULL){ // Se o contexto existir

                    // if(ctx->tree != NULL){
                    //     printf("Árvore K = 1, contexto: %s\n", context_str[K1]);
                    //     show_tree(ctx->tree->root, 1);
                    // }
                    Item** table = ctx->symb_table; // Obtém a tabela de símbolos
                    /*Pesquisa pelo código dentro do contexto*/
                    for(int i = 0; i < TABLE_SIZE; i++){
                        if(table[i] != NULL && table[i]->value->code.value == code->value && table[i]->value->code.length == code->length)
                        {
                            /*Se tiver encontrado o código dentro desse contexto, incremente o contador*/
                            found_symbol = table[i]->value;
                            break;
                        }
                    }

                    if(found_symbol != NULL){
                        if(strcmp(found_symbol->repr, RHO))/*Se o símbolo encontrado não for um 'rho'*/
                        {
                            // printf("\033[0;32m<<<<<<<<<<<<<<<<<<<<<<<<<< Encontrou o símbolo \"%s\" no contexto \"%s\" >>>>>>>>>>>>>>>>>>>\033[0m\n", found_symbol->repr, context_str[K1]);
                            fprintf(outfile, "%s", found_symbol->repr);
                            increment_item(table, found_symbol->repr);
                            code->value = code->length = 0;
                            update_context_str(context_str[K1], K1+1, found_symbol->repr);
                            rebuild_tree(ctx);

                            if(!strcmp(found_symbol->repr, " ")){
                                printf("\033[0;32mPalavra: %s\033[0m\n", word);
                                sprintf(word, "");
                                getchar();

                            }else{
                                strcat(word, found_symbol->repr);
                            }

                            explored++;
                            printf("explored: %d/%d\n", explored, file_size);

                        }else{
                            printf("Encontrou um 'rho' em K=1\n");
                            code->value = code->length = 0;
                            skip_k1 = true;
                        }
                        continue;
                    }else if(code->length < ctx->max_search_length){
                        // printf("\tContinue procurando em k = 1...\n");
                        continue;
                    }
                }else{ /*Se o contexto não existe ainda, crie*/
                    ctx = create_context(k1_table, context_str[K1]);
                    rebuild_tree(ctx);
                }
            }

            printf("Skip_k0: %d\n", skip_k0);
            //K = 0
            if(!skip_k0){
                if((found_symbol = search_code(k0_info.symb_table, code)) != NULL){
                    if(!strcmp(found_symbol->repr, RHO)){
                        code->length = code->value = 0;
                        skip_k0 = true;
                        skip_k1 = true;
                        printf("\033[0;35mEncontrou um rho! N(k=0) = %d\033[0m\n", eqprob_info.n_symb);

                        if(eqprob_info.n_symb == 1){
                            explored++;
                            printf("explored: %d/%d\n", explored, file_size);

                            Symbol** extracted_symbols = extract_symbols(&eqprob_info, TABLE_SIZE);                            
                            fprintf(outfile, "%s", extracted_symbols[0]->repr);

                            if(ctx != NULL){
                                add_to_context(ctx, extracted_symbols[0]->repr);
    
                                if(get_item(ctx->symb_table, RHO) != NULL){
                                    increment_item(ctx->symb_table, RHO);
                                }else{
                                    add_to_context(ctx, RHO);
                                }
                            }

                            if(!strcmp(extracted_symbols[0]->repr, " ")){
                                printf("\033[0;32mPalavra: %s\033[0m\n", word);
                                sprintf(word, "");
                                getchar();

                            }else{
                                strcat(word, extracted_symbols[0]->repr);
                            }

                            update_context_str(context_str[K1], K1+1, extracted_symbols[0]->repr);

                            add_to_context(&k0_info, extracted_symbols[0]->repr);

                            remove_item(eqprob_info.symb_table, extracted_symbols[0]->repr);
                            eqprob_info.n_symb--;

                            remove_item(k0_info.symb_table, RHO);
                            rebuild_tree(ctx); // Recontrói árvore de k = 1

                            k0_info.n_symb--;
                            rebuild_tree(&eqprob_info);
                            rebuild_tree(&k0_info);
                            skip_k0 = false;
                        }
                    }else{
                        skip_k1 = false;
                        if(ctx != NULL){
                            add_to_context(ctx, found_symbol->repr);

                            if(get_item(ctx->symb_table, RHO) != NULL){
                                increment_item(ctx->symb_table, RHO);
                            }else{
                                add_to_context(ctx, RHO);
                            }
                        }
                        printf("\033[0;32m<<<<<<<<<<<<<<<<<<<<< Mandando \033[0;31m\"%s\"\033[0m de K = 0 >>>>>>>>>>>>>>>>>>>>>>\033[0m\n", found_symbol->repr);
                        fprintf(outfile, "%s", found_symbol->repr);
                        increment_item(k0_info.symb_table, found_symbol->repr);

                        if(!strcmp(found_symbol->repr, " ")){
                            printf("\033[0;32mPalavra: %s\033[0m\n", word);
                            sprintf(word, "");
                            getchar();
                        }else{
                            strcat(word, found_symbol->repr);
                        }

                        update_context_str(context_str[K1], K1+1, found_symbol->repr);


                        rebuild_tree(&k0_info);
                        rebuild_tree(ctx); // Recontrói árvore de k = 1
                        code->length = code->value = 0;
                        explored++;
                        printf("explored: %d/%d\n", explored, file_size);

                    }
                    continue;
                }else if(code->length < k0_info.max_search_length){
                    continue;
                }
            }

            //K = -1
            if((found_symbol = search_code(eqprob_info.symb_table, code)) != NULL){
                explored++;
                printf("explored: %d/%d\n", explored, file_size);

                printf("\033[0;32m <<<<<<<<<<<<<<<<<<<<<<<<<<<<< Mandando \033[0;32m\"%s\"\033[0m de k = -1\033[0m >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", found_symbol->repr);
                fprintf(outfile, "%s", found_symbol->repr);
                add_to_context(&k0_info, found_symbol->repr);

                if(ctx != NULL){
                    add_to_context(ctx, found_symbol->repr);

                    if(get_item(ctx->symb_table, RHO) != NULL){
                        increment_item(ctx->symb_table, RHO);
                    }else{
                        add_to_context(ctx, RHO);
                    }
                }


                if(!strcmp(found_symbol->repr, " ")){
                    printf("\033[0;32mPalavra: %s\033[0m\n", word);
                    sprintf(word, "");
                    getchar();
                }else{
                    strcat(word, found_symbol->repr);
                }

                update_context_str(context_str[K1], K1+1, found_symbol->repr);
                

                if(get_item(k0_info.symb_table, RHO) != NULL){
                    increment_item(k0_info.symb_table, RHO);
                }else{
                    add_to_context(&k0_info, RHO);
                }
                remove_item(eqprob_info.symb_table, found_symbol->repr);
                eqprob_info.n_symb--;

                skip_k0 = false;
                skip_k1 = false;

                code->value = code->length = 0;
            }

            if(ctx != NULL)rebuild_tree(ctx); // Recontrói árvore de k = 1
            rebuild_tree(&eqprob_info);
            rebuild_tree(&k0_info);

        }
    }

    fclose(file);
    fclose(outfile);

    destroy_tree(eqprob_info.tree);
    destroy_tree(k0_info.tree);

    destroy_map(eqprob_info.symb_table, TABLE_SIZE);
    destroy_map(k0_info.symb_table, TABLE_SIZE);
}