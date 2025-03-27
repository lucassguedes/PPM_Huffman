#include "hash.h"

int hash(char* key){
	/**Interpretando a string como um número natural.*/
	int n = strlen(key);
	int k = 0;
	for(int i = 0; i < n; i++){
		k += (int)key[i] * pow(128, n - i - 1);
	}

	/*Aplicando o método da multiplicação*/
	return (int)floor(M*(k*A - floor(k*A)));
}