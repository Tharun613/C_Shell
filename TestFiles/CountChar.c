#include <stdio.h>

// count characters program


int main(){
	char string[100];
	fgets(string,100,stdin);
	int length = 0;
	while(string[length] != '\n'){
		length++;
	}
	printf("%d",length);

}