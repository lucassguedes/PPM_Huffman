#include "ppm.h"

int g_context_limit;

int limits[] = {4096, 4096, 4096, 4096, 2048};


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
    while (bits_to_write > *outbuffer_length)
    {
        bits_to_ignore = bits_to_write - *outbuffer_length; // bits_to_ignore -> número de bits que serão deixados para o próximo byte

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

    if (*outbuffer_length == 0)
    {
        fputc(*outbuffer, outfile);
        *outbuffer = 0;
        *outbuffer_length = 8;
    }
}

void rebuild_tree(ContextInfo *ctx)
{

    destroy_tree(ctx->tree);
    if (ctx->n_symb)
    {
        Symbol **extracted_symbols = NULL;
        /*Reconstruindo árvore de contexto k=0*/
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

void add_to_context(ContextInfo *ctx, char *repr)
{
    Symbol new_symbol;
    new_symbol.repr = (char *)malloc(sizeof(char) * (strlen(repr) + 1));
    new_symbol.counter = 1;
    new_symbol.code.length = new_symbol.code.value = 0;
    strcpy(new_symbol.repr, repr);
    add_item(ctx->symb_table, new_symbol);
    ctx->n_symb++;
    free(new_symbol.repr);
}

void update_context_str(char context_str[], int k, char *buffer)
{
    const int str_size = strlen(context_str);

    if (!strcmp(context_str, EMPTY_CONTEXT))
    {
        sprintf(context_str, "%c", buffer[0]);
        return;
    }

    if (str_size == k)
    {
        for (int i = 0; i < str_size - 1; i++)
        {
            context_str[i] = context_str[i + 1];
        }
        context_str[str_size - 1] = buffer[0];
        context_str[str_size] = '\0';
        return;
    }

    context_str[str_size] = buffer[0];
    context_str[str_size + 1] = '\0';
}

ContextInfo *get_context(ContextInfo **contexts, char *key)
{
    const int index = hash(key);

    ContextInfo *ctx = contexts[index];

    while (ctx != NULL)
    {
        if (!strcmp(ctx->name, key))
        {
            return ctx;
        }
    }

    return NULL;
}

ContextInfo *create_context(ContextInfo **contexts, char *key)
{
    const int index = hash(key);

    ContextInfo *new_context = (ContextInfo *)malloc(sizeof(ContextInfo));

    initialize_ppm_table(new_context);

    new_context->name = key;

    if (contexts[index] == NULL)
    {
        contexts[index] = new_context;
        return contexts[index];
    }

    ContextInfo *tmp = contexts[index];

    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }

    tmp->next = new_context;

    return new_context;
}

/**
 * Realiza  a  busca  pelo  código  dentro  do  contexto  e,  ao
 * final, indica se o loop se devemos efetuar um continue (true),
 * ou dar prosseguimento à busca nos próximos contextos.
 */
bool contextual_search_symbol(ContextInfo **contexts, int n_contexts[],
                              int K, uint8_t *outbuffer, int *outbuffer_length,
                              char *context_str, FILE *outfile,
                              char *buffer, int *explored)
{

    if (!strcmp(context_str, EMPTY_CONTEXT) || strlen(context_str) < K + 1)
    {
        update_context_str(context_str, K + 1, buffer);
        return false;    
    }   

    ContextInfo *ctx;
    ctx = get_context(contexts, context_str);

    if (ctx == NULL) // Se o contexto não existe
    {

        if(n_contexts[K] > g_context_limit){
            // printf("\033[0;31mOpaa!!Muitos contextos criados!! Mais de 4096!\033[0m\n");
            
            /*Limpando o contexto*/
            int deleted = 0;
            for(int i = 0; i < TABLE_SIZE; i++){
                ContextInfo* current_ctx = contexts[i];
                contexts[i] = NULL;
                while(current_ctx != NULL){
                    ContextInfo * next = current_ctx->next;

                    destroy_tree(current_ctx->tree);
                    destroy_map(current_ctx->symb_table, TABLE_SIZE);
                    free(current_ctx);
                    current_ctx = next; 
                    // deleted++;
                }

                // if(deleted > 1000){
                //     break;
                // }
            }

            // n_contexts[K] -= deleted;
            n_contexts[K] = 0;
        }
        /*Crie o contexto e adicione o novo símbolo*/
        ctx = create_context(contexts, context_str);
        add_to_context(ctx, buffer);
        add_to_context(ctx, RHO);
        rebuild_tree(ctx);
        update_context_str(context_str, K + 1, buffer);               // Atualizamos a string de contexto
        n_contexts[K]++;
        return false; // Deve continuar a busca nos contextos seguintes
    }

    char code_str[30];
    Symbol *sb;

    // printf("O contexto \"%s\" já existe!\n", context_str);
    sb = get_item(ctx->symb_table, buffer); /*Verificando se o símbolo atual está neste contexto*/

    if (sb != NULL) // Encontrou o símbolo dentro do contexto atual!
    {
        // printf("Encontrou o símbolo \033[0;32m\"%s\"\033[0m no contexto \033[0;32m\"%s\"\033[0m - Código: ", buffer, context_str);
        get_bin_str(sb, code_str);
        // printf("%s\n", code_str);
        increment_item(ctx->symb_table, buffer);                      // Incrementando o contador do símbolo
        write_code_to_file(outfile, sb, outbuffer, outbuffer_length); // Escrevemos o código no arquivo
        update_context_str(context_str, K + 1, buffer);               // Atualizamos a string de contexto
        rebuild_tree(ctx);
        (*explored)++;
        return true;
    }
    /*Caso não tenha encontrado o código neste contexto*/
    Symbol *rho = get_item(ctx->symb_table, RHO); // Obtém o 'rho'
    get_bin_str(rho, code_str);
    // printf("Codificando 'rho' para k = %d - Código: %s\n", K + 1, code_str);
    write_code_to_file(outfile, rho, outbuffer, outbuffer_length); // Escrevemos o código de 'rho' na saída
    increment_item(ctx->symb_table, RHO);                          // Incrementa o contador de 'rho'
    add_to_context(ctx, buffer);                                   // Adiciona o novo símbolo ao contexto
    rebuild_tree(ctx);

    update_context_str(context_str, K + 1, buffer);

    return false;
}

void compress(char *input_filepath, char *output_filepath, int K)
{

    g_context_limit = limits[K - 1];

    printf("g_context_limit: %d\n", g_context_limit);

    /*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
    ContextInfo k0_info;
    ContextInfo eqprob_info;

    /***
     * Dicionário  que  contém  os  contextos  iguais  a 1.
     * Cada posição representa um contexto e é acessada por
     * uma  string,  cada  uma contém um mapa de símbolos e
     * seus respectivos códigos.
     */

    ContextInfo ***contextual_tables;

    int n_contexts[K]; // Número de contextos criados em cada K


    contextual_tables = (ContextInfo***)malloc(sizeof(ContextInfo**)*K);

    for(int k = 0; k < K; k++){
        n_contexts[k] = 0;
        contextual_tables[k] = (ContextInfo**)malloc(sizeof(ContextInfo*)*TABLE_SIZE);
        for (int i = 0; i < TABLE_SIZE; i++)
        {   
            contextual_tables[k][i] = NULL;
        }
    }

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

    uint8_t outbuffer = 0;    /*Byte a ser mandado para a saída.*/
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

    while ((c = fgetc(file)) != EOF)
    {
        // printf("########################################################\n");
        sprintf(buffer, "%c", c);

        // printf("Símbolo atual: \033[0;31m\"%s\"\033[0m\n", buffer);
        // printf("Explored: %d / %d\n", explored, file_size);

        // update_context_str(context_str[K2], K2+1, buffer);
        // update_context_str(context_str[K3], K3+1, buffer);
        // update_context_str(context_str[K4], K4+1, buffer);
        // update_context_str(context_str[K5], K5+1, buffer)


        /*Pesquisando nos contextos K a K1*/
        bool must_continue = false;
        for(int k = K-1; k >= K1; k--){
            if (contextual_search_symbol(contextual_tables[k], n_contexts, k, &outbuffer, &outbuffer_length, context_str[k], outfile, buffer, &explored))
            {
                must_continue = true;
                break;
            }
        }

        if(must_continue) continue;

        // K = 0
        sb = get_item(k0_info.symb_table, buffer);
        if (sb != NULL)
        {
            /** Se o símbolo atual FOI encontrado na tabela k=0 */
            /*Icrementa o contador do símbolo*/
            increment_item(k0_info.symb_table, buffer);
            get_bin_str(sb, code_str);
            // printf("Símbolo '%s' encontrado na tabela k=0...\033[0;32mcom código %s\033[0m\n", sb->repr, code_str);
            /*Manda o código do símbolo para a saída*/
            write_code_to_file(outfile, sb, &outbuffer, &outbuffer_length);
            n_bits += sb->code.length;
            /*Reconstruindo a árvore de k=0*/
            rebuild_tree(&k0_info);
            explored++;
            continue;
        }

        // k = -1
        sb = get_item(eqprob_info.symb_table, buffer); // Procurando símbolo na tabela k = -1
        if (sb != NULL)                                // Se o símbolo estiver na tabela k = -1
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
                // printf("Codificando um 'rho' de K=0 na saída - Código: %s\n", code_str);
                write_code_to_file(outfile, rho, &outbuffer, &outbuffer_length);
                n_bits += rho->code.length;
            }
            else
            { /*Se o símbolo 'rho' ainda não estiver na tabela k=0*/
                /*Adiciona 'rho' à tabela*/
                add_to_context(&k0_info, RHO);
            }

            /*Mandando código do símbolo para a saída (a partir da tabela de equiprobabilidade)*/
            if (eqprob_info.n_symb > 1)
            {
                get_bin_str(sb, code_str);
                // printf("Codificando \"%s\" a partir da tabela k = -1 - Código: %s\033[0m\n", sb->repr, code_str);
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


    for(int k = 0; k < K; k++){
        for (int i = 0; i < TABLE_SIZE; i++)
        {   
            if(contextual_tables[k][i] != NULL){
                destroy_tree(contextual_tables[k][i]->tree);
                destroy_map(contextual_tables[k][i]->symb_table, TABLE_SIZE); 
                free(contextual_tables[k][i]);
            }
        }
        free(contextual_tables[k]);
    }
    free(contextual_tables);

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

bool contextual_search_code(FILE* outfile, ContextInfo** contexts, int n_contexts[], Code* code, Symbol** found_symbol,
                            bool* skip, char* context_str, int K, int* explored){
    

    // printf("context_str (k=%d): %s, skip: %d\n", K+1, context_str, *skip);
    if((*skip) || (!strcmp(context_str, EMPTY_CONTEXT) || strlen(context_str) < K + 1)){
        return false;
    }

    ContextInfo* ctx = NULL;

    ctx = get_context(contexts, context_str);

    if(ctx == NULL){
        // printf("\033[0;31mO contexto \"%s\" não existe!\033[0m\n", context_str);
        /*Se o contexto não existe ainda, crie*/
        
        if(n_contexts[K] > g_context_limit){
            // printf("\033[0;31mOpaa!!Muitos contextos criados!! Mais de 4096!\033[0m\n");
            
            /*Limpando o contexto*/
            int deleted = 0;
            for(int i = 0; i < TABLE_SIZE; i++){
                ContextInfo* current_ctx = contexts[i];
                contexts[i] = NULL;
                while(current_ctx != NULL){
                    ContextInfo * next = current_ctx->next;

                    destroy_tree(current_ctx->tree);
                    destroy_map(current_ctx->symb_table, TABLE_SIZE);
                    free(current_ctx);
                    current_ctx = next; 
                    // deleted++;
                }

                // if(deleted > 1000){
                //     break;
                // }
            }

            // n_contexts[K] -= deleted;
            n_contexts[K] = 0;
        }
        n_contexts[K]++;
        ctx = create_context(contexts, context_str);
        rebuild_tree(ctx);
        return false;
    }

    // Se a string de contexto não for vazia
    /* Verificando a existência do contexto*/
    (*found_symbol) = NULL;
    bool is_rho = false;

    // Se o contexto existir

    // if(ctx->tree != NULL){
    //     printf("Árvore K = 2, contexto: %s\n", context_str);
    //     show_tree(ctx->tree->root, 1);
    // }
    Item **table = ctx->symb_table; // Obtém a tabela de símbolos
    /*Pesquisa pelo código dentro do contexto*/
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (table[i] != NULL && table[i]->value->code.value == code->value && table[i]->value->code.length == code->length)
        {
            /*Se tiver encontrado o código dentro desse contexto, incremente o contador*/
            (*found_symbol) = table[i]->value;
            break;
        }
    }

    if ((*found_symbol) != NULL)
    {
        if (strcmp((*found_symbol)->repr, RHO)) /*Se o símbolo encontrado não for um 'rho'*/
        {
            // printf("\033[0;32m<<<<<<<<<<<<<<<<<<<<<<<<<< Encontrou o símbolo \"%s\" no contexto \"%s\" >>>>>>>>>>>>>>>>>>>\033[0m\n", (*found_symbol)->repr, context_str);
            fprintf(outfile, "%s", (*found_symbol)->repr);
            increment_item(table, (*found_symbol)->repr);
            code->value = code->length = 0;
            update_context_str(context_str, K + 1, (*found_symbol)->repr);
            rebuild_tree(ctx);

            (*explored)++;
        }
        else
        {
            // printf("Encontrou um 'rho' em K=%d\n", K + 1);
            code->value = code->length = 0;
            (*skip) = true;
        }
        return true;
    }
    
    if (code->length < ctx->max_search_length) /*Se ainda não explorou todos, continue a pesquisa nesse contexto*/
    {
        return true;
    }

    return false;
}


void update_context_symbols(ContextInfo* ctx, Symbol* symbol){
    if (ctx != NULL)
    {
        add_to_context(ctx, symbol->repr);

        if (get_item(ctx->symb_table, RHO) != NULL)
        {
            increment_item(ctx->symb_table, RHO);
        }
        else
        {
            add_to_context(ctx, RHO);
        }
        rebuild_tree(ctx); // Recontrói árvore de k = 1
    }

}   


void decompress(char *input_filepath, char *output_filepath, int K)
{
    /*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
    ContextInfo k0_info;
    ContextInfo eqprob_info;

    g_context_limit = limits[K - 1];

    printf("g_context_limit: %d\n", g_context_limit);

    int n_contexts[K]; // Número de contextos criados em cada K

    
    ContextInfo ***contextual_tables = NULL;
    
    contextual_tables = (ContextInfo***)malloc(sizeof(ContextInfo**)*K);


    for(int i = 0; i < K; i++){
        n_contexts[i] = 0;
        contextual_tables[i] = (ContextInfo**)malloc(sizeof(ContextInfo*)*TABLE_SIZE);
        for (int j = 0; j < TABLE_SIZE; j++){
            contextual_tables[i][j] = NULL;
        }
    }

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
    bool* skip = (bool*)malloc(sizeof(bool)*K);

    char context_str[5][30];

    strcpy(context_str[K1], EMPTY_CONTEXT);
    strcpy(context_str[K2], EMPTY_CONTEXT);
    strcpy(context_str[K3], EMPTY_CONTEXT);
    strcpy(context_str[K4], EMPTY_CONTEXT);
    strcpy(context_str[K5], EMPTY_CONTEXT);
    
    
    ContextInfo* current_contexts[K]; //Contextos atuais, para cada valor de K (1,2,3,4,5)
    

    char code_str[30];
    bool must_continue = false;
    while (explored < file_size)
    {

        byte = fgetc(file);

        // printf("Lendo byte: %d\n", byte);
        bits_to_read = 8;

        while (bits_to_read)
        {
            if (explored == file_size)
            {
                break;
            }

            code->value = code->value << 1;
            code->value |= (byte & (int)pow(2, bits_to_read) - 1) >> (bits_to_read - 1);
            code->length++;
            bits_to_read--;

            Symbol xs;
            xs.code = *code;

            get_bin_str(&xs, code_str);
            // printf("\033[0;32mCode: %s\033[0m\n", code_str);

            // K = 2
            bool contextual_must_continue = false;
            for(int k = K - 1; k >= K2; k--){
                skip[k] = (skip[k-1]) ? skip[k-1] : skip[k]; 

                must_continue = contextual_search_code(outfile, contextual_tables[k], n_contexts, code, &found_symbol, &skip[k], context_str[k], k, &explored);

                if(must_continue){
                    if(k < K - 1){
                        for(int j = k + 1; j < K; j++){
                            if(found_symbol != NULL && strcmp(found_symbol->repr, RHO)){
                                // printf("Atualizando K = %d, com o símbolo \"%s\"\n", j, found_symbol->repr);
                                update_context_symbols(current_contexts[j], found_symbol);
                                update_context_str(context_str[j], j + 1, found_symbol->repr);
                                skip[j] = false;
                            }
                        }
                    }
                    contextual_must_continue = true;
                    break;
                }
                current_contexts[k] = get_context(contextual_tables[k], context_str[k]);

                if(explored == file_size) contextual_must_continue = true;
            }

            if(contextual_must_continue) continue;


            // K = 1                if(explored == file_size) contextual_must_continue = true;

            must_continue = contextual_search_code(outfile, contextual_tables[K1], n_contexts, code, &found_symbol, &skip[K1], context_str[K1], K1, &explored);

            if(must_continue){
                for(int j = K2; j < K; j++){
                    if(found_symbol != NULL && strcmp(found_symbol->repr, RHO)){
                        // printf("Atualizando K = 2, com o símbolo \"%s\"\n", found_symbol->repr);
                        update_context_symbols(current_contexts[j], found_symbol);
                        update_context_str(context_str[j], j + 1, found_symbol->repr);
                        skip[j] = false;
                    }
                }
                continue;
            }

            current_contexts[K1] = get_context(contextual_tables[K1], context_str[K1]);

            

            // printf("Skip_k0: %d\n", skip_k0);
            // K = 0
            if (!skip_k0)
            {
                if ((found_symbol = search_code(k0_info.symb_table, code)) != NULL)
                {
                    if (!strcmp(found_symbol->repr, RHO))
                    {
                        code->length = code->value = 0;
                        skip_k0 = true;
                        for(int k = 0; k < K; k++){
                            skip[k] = true;
                        }

                        // printf("\033[0;35mEncontrou um rho! N(k=0) = %d\033[0m\n", eqprob_info.n_symb);

                        if (eqprob_info.n_symb == 1)
                        {
                            explored++;
                            // printf("explored: %d/%d\n", explored, file_size);

                            Symbol **extracted_symbols = extract_symbols(&eqprob_info, TABLE_SIZE);
                            fprintf(outfile, "%s", extracted_symbols[0]->repr);

                            for(int k = 0; k < K; k++){
                                update_context_symbols(current_contexts[k], extracted_symbols[0]);
                                update_context_str(context_str[k], k + 1, extracted_symbols[0]->repr);
                            }

                            add_to_context(&k0_info, extracted_symbols[0]->repr);

                            remove_item(eqprob_info.symb_table, extracted_symbols[0]->repr);
                            eqprob_info.n_symb--;

                            remove_item(k0_info.symb_table, RHO);

                            k0_info.n_symb--;
                            rebuild_tree(&eqprob_info);
                            rebuild_tree(&k0_info);
                            skip_k0 = false;
                            for(int k = 0; k < K; k++){
                                skip[k] = false;
                            }

                        }
                    }
                    else
                    {
                        for(int k = 0; k < K; k++){
                            skip[k] = false;
                        }

                        for(int k = 0; k < K; k++){
                            update_context_symbols(current_contexts[k], found_symbol);
                            update_context_str(context_str[k], k + 1, found_symbol->repr);
                        }

                        // printf("\033[0;32m<<<<<<<<<<<<<<<<<<<<< Mandando \033[0;31m\"%s\"\033[0m de K = 0 >>>>>>>>>>>>>>>>>>>>>>\033[0m\n", found_symbol->repr);
                        fprintf(outfile, "%s", found_symbol->repr);
                        increment_item(k0_info.symb_table, found_symbol->repr);


                        rebuild_tree(&k0_info);
                        code->length = code->value = 0;
                        explored++;
                        // printf("explored: %d/%d\n", explored, file_size);
                    }
                    continue;
                }
                else if (code->length < k0_info.max_search_length)
                {
                    continue;
                }
            }

            // K = -1
            if ((found_symbol = search_code(eqprob_info.symb_table, code)) != NULL)
            {
                explored++;
                // printf("explored: %d/%d\n", explored, file_size);

                // printf("\033[0;32m <<<<<<<<<<<<<<<<<<<<<<<<<<<<< Mandando \033[0;32m\"%s\"\033[0m de k = -1\033[0m >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", found_symbol->repr);
                fprintf(outfile, "%s", found_symbol->repr);
                add_to_context(&k0_info, found_symbol->repr);

                for(int k = 0; k < K; k++){
                    // printf("k = %d\n", k);
                    update_context_symbols(current_contexts[k], found_symbol);
                    update_context_str(context_str[k], k + 1, found_symbol->repr);
                }


                if (get_item(k0_info.symb_table, RHO) != NULL)
                {
                    increment_item(k0_info.symb_table, RHO);
                }
                else
                {
                    add_to_context(&k0_info, RHO);
                }
                remove_item(eqprob_info.symb_table, found_symbol->repr);
                eqprob_info.n_symb--;

                skip_k0 = false;
                for(int k = 0; k < K; k++){
                    skip[k] = false;
                }



                code->value = code->length = 0;
            }

            
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