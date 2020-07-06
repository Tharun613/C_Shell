#include <stdio.h>

//choose Name

int main(){
	char* Names[] = {
		"Harry Potter\0",
		"Hermione Granger\0",
		"Ronald Weasley\0",
		"Dumbledore\0",
		"Severus Snape\0",
		"Luna Lovegood\0"
		"Tom Riddle\0"

	};
	int defaultname = 0;
	scanf("%d",&defaultname);
	defaultname = (defaultname % 7);
	printf("%s",Names[defaultname]);
	
}