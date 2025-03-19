#include <stdio.h>
#include <stdlib.h>



int main(int argc, char** argv){

	
	if(argc < 2){
		printf("\033[0;31mError:\033[0m Insufficient parameters.\n");
		return -1;
	}
	

	printf("Instance name: %s\n", argv[1]);


	return 0;
}
