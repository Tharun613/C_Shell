#include <stdio.h>

// Greet name 

int main(){
	char string[50];
	fgets(string,50,stdin);
	int length = 0;
	while(string[length] != '\0'){
		if(string[length] == '\n'){
			string[length] = '\0';
		}
		length++;
	}
	printf("Hello %s",string);
}