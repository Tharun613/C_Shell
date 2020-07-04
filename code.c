#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAXLENGTH 100

int changeDirectory(char *string){
	chdir(string);
	return 0;
}

int printDirectory(char *string){
	char c[100];
	getcwd(c,sizeof(c));
	printf("current Working Directory: %s\n",c);
}

int makeDirectory(char *string){
	struct stat st = {0};

	if(stat(string, &st) == -1){
		mkdir(string,0700);
	}

	return 0;
}

int removeDirectory(char *string){
	rmdir(string);
	return 0;
}

int countPipes(char* string){
	int i = 0;
	int ret = 0;
	while(string[i]!='\0'){
		if(string[i] == '|')
			ret++;
		i++;
	}
	return ret;
}

char* removeSpaces(char* string){
	puts("Inside remove Spaces");
	int i = 0;
	int numChars = 0;
	while(string[i] != '\0'){
		
		if(string[i] == ' '){
			i++;
			continue;
		}
		else{
			i++;
			numChars++;
		}
	}
	char* newString = (char*)malloc((numChars+1)*sizeof(char));
	i = 0;
	int j = 0;
	while(string[i] != '\0'){
		
		if(string[i] == ' '){
			i++;
			continue;
		}
		else{
			newString[j++] = string[i++];
		}
	}
	newString[j] = '\0';

	return newString;
}

char* getDescriptors(char* command,int* readEnd,int* writeEnd){
	//takes in the command and returns the name of the executable.
	int i = 0;
	*readEnd  =  -1;
	*writeEnd =  -1;

	char* withoutSpaces = removeSpaces(command);

	int lessthan = -1,greaterthan = -1,end = -1;
	i = 0;

	while(1){

		if(withoutSpaces[i] == '<'){
			lessthan = i;
			withoutSpaces[lessthan] = '\0';
		}
		else if(withoutSpaces[i] == '>'){
			greaterthan = i;
			withoutSpaces[greaterthan] = '\0';
		}
		else if(withoutSpaces[i] == '\0'){
			end = i;
			break;
		}
		i++;

	}
	if(lessthan != -1){
		*readEnd = open(&withoutSpaces[lessthan+1],O_RDONLY);
	}
	if(greaterthan != -1){
		*writeEnd = open(&withoutSpaces[greaterthan+1],O_WRONLY | O_CREAT);
		puts("Opened the file required");
		dup2(*writeEnd,1);
		puts("There you are!!!");
		close(1);
	}
	return strdup(withoutSpaces);
	
}
void execute(char* command,int readEnd,int writeEnd){
 	int pid = fork();
 	
 	// Here command is the name of the executable not the whole command.

 	if(pid == 0){
 		//dup2(writeEnd,STDOUT_FILENO);
 		dup2(readEnd,0);
 		char* array[] = {command,NULL};
 		puts("Tharun is great");
 		execvp(array[0],array);
 	}
 	else{

 		wait(NULL);
 		puts("\nChild process terminated!!!");

 	}
}
void parseExecutable(char* string){
	char* withoutSpaces = removeSpaces(string);
	int pipes = countPipes(withoutSpaces);
	if(pipes == 0){
		puts("zero pipes");
		int readEnd = 0,writeEnd = 1;
		char* executableName = getDescriptors(withoutSpaces,&readEnd,&writeEnd);
		
		if(readEnd < 0)readEnd = 0;
		if(writeEnd < 0)writeEnd = 1;
		printf("ReadEnd %d WriteEnd %d\n",readEnd,writeEnd);
		execute(executableName,readEnd,writeEnd);

	}
	else if(pipes == 1){
		puts("1 pipe");
	}
	else{
		puts("more than 1 pipe");
	}
}
// cd
// pwd
// rmdir
// mkdir
// exit
int runCommand(char* string){
	char* command = strdup(string);
	char* token = strtok(command," ");

	if(strcmp(token,"cd") == 0){
		puts("Cd");
		token = strtok(NULL," ");
		changeDirectory(token);
	}
	else if(strcmp(token,"pwd") == 0){
		puts("pwd");
		token = strtok(NULL," ");
		printDirectory(token);
	}
	else if(strcmp(token,"rmdir") == 0){
		puts("rmdir");
		token = strtok(NULL," ");
		removeDirectory(token);
	}
	else if(strcmp(token,"mkdir") == 0){
		puts("mkdir");
		token = strtok(NULL," ");
		makeDirectory(token);
	}
	else if(strcmp(token,"exit") == 0){
		puts("Exit");
		return 0;
	}
	else{
		puts("executable");
		int i = 0;
		parseExecutable(string);
	}
	return 1;
}
int main(){
	int running = 1;

	while(running){
		printf("Shell>>");
		char command[MAXLENGTH];
		fgets(command,MAXLENGTH,stdin);
		int i = 0;
		
		while(command[i] != '\n')
			i++;
		command[i] = '\0';


		running = runCommand(command);
	}
}
