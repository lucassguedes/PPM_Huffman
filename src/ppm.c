#include "ppm.h"

void initialize_equiprob_table(ContextInfo* ctx){
    /*A tabela k=-1 inicialmente deve conter todos os símbolos do alfabeto, com contadores iguais a 1*/
	ctx->n_symb = 27;
	ctx->symb_table = (Item**)malloc(sizeof(Item*)*TABLE_SIZE);
    ctx->tree = NULL;


	for(int i = 0; i < ctx->n_symb; i++) ctx->symb_table[i] = NULL;


	/*Adicionando símbolos do alfabeto*/
	Symbol s;
	s.repr = (char*)malloc(sizeof(char)*2);
	strcpy(s.repr, " ");
	s.counter = 1;
	
	add_item(ctx->symb_table, s);

	strcpy(s.repr, "a");
	for(int i = 0; i < 26; i++){
		add_item(ctx->symb_table, s);
		s.repr[0]++;
	}

    free(s.repr);

	/*Extraindo símbolos da tabela de símbolos - esta operação é feita sempre que a árvore
	  precisa ser reconstruída.
	*/
	Symbol** all_symbols = extract_symbols(ctx, TABLE_SIZE);
	ctx->tree = create_tree(all_symbols, 27);
	/*Atualiza os códigos dos símbolos de acordo com a estrutura da árvore*/
	set_codes(ctx->tree, all_symbols, 27);

    free(all_symbols);
}

void initialize_ppm_table(ContextInfo* ctx){
    ctx->n_symb = 0;
	ctx->symb_table = (Item**)malloc(sizeof(Item*)*TABLE_SIZE);
    ctx->tree = NULL;

	for(int i = 0; i < ctx->n_symb; i++) ctx->symb_table[i] = NULL;
}

void compress(char* input_filepath, char* output_filepath){

	/*Cria duas tabelas, uma para o contexto k=-1 (posição 0) e outra para k=0 (posição 1)*/
	ContextInfo contexts[2];

    initialize_equiprob_table(&contexts[EQPROB_TABLE]);
    initialize_ppm_table(&contexts[K0_TABLE]);

    FILE* file = fopen(input_filepath, "r");

    char c;
    int k;
    char buffer[5];
    Symbol* sb;
    Symbol** extracted_symbols;
    Symbol new_symbol;

    char outputstring[3000];
    char code_str[30];

    sprintf(outputstring, "");

    while((c = fgetc(file)) != EOF){
        k = K0_TABLE;
        sprintf(buffer, "%c", c);
        sb = get_item(contexts[k].symb_table, buffer);
        if(sb != NULL){
            printf("Símbolo '%s' encontrado na tabela k=0...\n", sb->repr);
            increment_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, buffer);
            /*Destruindo a árvore de k=0*/
            destroy_tree(contexts[K0_TABLE].tree);

            /*Reconstruindo a árvore de k=0*/
            extracted_symbols = extract_symbols(&contexts[K0_TABLE], TABLE_SIZE);
            contexts[K0_TABLE].tree = create_tree(extracted_symbols, contexts[K0_TABLE].n_symb);
            free(extracted_symbols);
            extracted_symbols = NULL;
        }
        else{
            printf("Símbolo '%c' não encontrado na tabela k=0, chaveando para k=-1...\n", c);
            k = EQPROB_TABLE;
            sb = get_item(contexts[k].symb_table, buffer);
            if(sb != NULL){
                printf("Símbolo %s encontrado na tabela k=-1... (N = %d)\n", buffer, contexts[EQPROB_TABLE].n_symb);
                
                if(contexts[EQPROB_TABLE].n_symb > 1){
                    printf("\t\tMandando código de %s para a saída\n", sb->repr);
                    get_bin_str(sb, code_str);
                    strcat(outputstring, code_str);
                }

                /*Adicionando o símbolo na tabela k=0 */     
                new_symbol.repr = (char*)malloc(sizeof(char)*(strlen(buffer) + 1));
                strcpy(new_symbol.repr, buffer);

                add_item(contexts[K0_TABLE].symb_table, new_symbol);
                contexts[K0_TABLE].n_symb++;

                free(new_symbol.repr);
        
                sb = get_item(contexts[K0_TABLE].symb_table, "rho");
                /*Adicionando 'rho' na tabela k=0 (ou incrementando)*/
                if(sb != NULL){
                    increment_item(contexts[K0_TABLE].symb_table, TABLE_SIZE, "rho");
                    printf("Aqui. Código do rho: %d\n", sb->code.value);
                    get_bin_str(sb, code_str);
                    strcat(outputstring, code_str); 
                    printf("\t\tMandando código de rho para a saída\n");
                }else{
                    new_symbol.repr = "rho";
                    new_symbol.counter = 1;
                    new_symbol.code.value = 0;
                    new_symbol.code.length = 0;
                    add_item(contexts[K0_TABLE].symb_table, new_symbol);
                    contexts[K0_TABLE].n_symb++;
                }

                /*Removendo símbolo da tabela k=-1*/
                remove_item(contexts[EQPROB_TABLE].symb_table, TABLE_SIZE, buffer);
                contexts[EQPROB_TABLE].n_symb--;
                /*Reconstruindo árvores*/
                destroy_tree(contexts[EQPROB_TABLE].tree);
                
                if(contexts[EQPROB_TABLE].n_symb){
                    extracted_symbols = extract_symbols(&contexts[EQPROB_TABLE], TABLE_SIZE);
                    contexts[EQPROB_TABLE].tree = create_tree(extracted_symbols, contexts[EQPROB_TABLE].n_symb);
                    free(extracted_symbols);
                }else{
                    contexts[EQPROB_TABLE].tree = NULL;
                }


                destroy_tree(contexts[K0_TABLE].tree);

                if(contexts[K0_TABLE].n_symb){
                    extracted_symbols = extract_symbols(&contexts[K0_TABLE], TABLE_SIZE);
                    contexts[K0_TABLE].tree = create_tree(extracted_symbols, contexts[K0_TABLE].n_symb);
                    free(extracted_symbols);
                    extracted_symbols = NULL;
                }else{
                    contexts[EQPROB_TABLE].tree = NULL;
                }

            }
        }
    }

    printf("Saída: %s\n", outputstring);

    fclose(file);
    destroy_tree(contexts[EQPROB_TABLE].tree);
    destroy_tree(contexts[K0_TABLE].tree);

    destroy_map(contexts[EQPROB_TABLE].symb_table, TABLE_SIZE);
    destroy_map(contexts[K0_TABLE].symb_table, TABLE_SIZE);
    
}