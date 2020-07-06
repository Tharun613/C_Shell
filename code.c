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
#define DEBUG 1

void red() {
  printf("\033[1;31m");
}
void reset () {
  printf("\033[0m");
}

int changeDirectory(char *string){
	chdir(string);
	return 0;
}

int printDirectory(){
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
		if(DEBUG){
			red();
			printf("\t\tOpened readEnd *%s*\n",&withoutSpaces[lessthan+1]);
			reset();
		}
	}
	if(greaterthan != -1){
		*writeEnd = open(&withoutSpaces[greaterthan+1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		if(DEBUG){
			red();
			printf("\t\tOpened writeEnd *%s*\n",&withoutSpaces[greaterthan+1]);
			reset();
		}

	}
	return strdup(withoutSpaces);
	
}
void execute(char* command,int readEnd,int writeEnd){
 	
 	int std_output = dup(1);
 	int std_input  = dup(0);

 	if(DEBUG){
 		red();
 		printf("\tExecuting execute Function\n");
 		reset();
 	}
 	int pid = fork();
 	
 	// Here command is the name of the executable not the whole command.

 	if(pid == 0){
 		//dup2(writeEnd,STDOUT_FILENO);
 		
 		reset();
 		
 		


 		dup2(readEnd,0);
 		dup2(writeEnd,1);


 		
 		char* array[] = {command,NULL};
 		

 		execvp(array[0],array);
 	}
 	else{

 		wait(NULL);

 		if(DEBUG){
 			red();
 			printf("\n\t\tChild Process Terminated\n");
 			reset();
 		}
 		

 		dup2(std_output,STDOUT_FILENO);
 		dup2(std_input,STDIN_FILENO);
 	}
}
void parseExecutable(char* string){
	if(DEBUG){
		red();
		printf("\tExecuting parseExecutable Function\n");
		
	}
	char* withoutSpaces = removeSpaces(string);
	
	int pipes = countPipes(withoutSpaces);
	
	if(DEBUG){
		printf("\t\tAfter removing space *%s*\n",withoutSpaces);
		printf("\t\tNumber of pipes : %d\n",pipes);
		reset();
	}

	if(pipes == 0){
		
		int readEnd = 0,writeEnd = 1;
		char* executableName = getDescriptors(withoutSpaces,&readEnd,&writeEnd);
		
		if(readEnd < 0)readEnd = 0;
		if(writeEnd < 0)writeEnd = 1;

		if(DEBUG){
			red();
			printf("\t\tRead :%d,Write :%d,Executable :*%s*\n",readEnd,writeEnd,executableName);
			reset();
		}

		execute(executableName,readEnd,writeEnd);

	}
	else{
		char* c = strdup(string);
		char* token = strtok(c,"|");

		int pipe1[2],pipe2[2];
		if(pipe(pipe1) == -1){
			puts("PIPE1 CREATION FAILED!!!");
		}
		if(pipe(pipe2) == -1){
			puts("PIPE2 CREATION FAILED!!!");
		}

		//THIS BLOCK EXECUTES START
		int readEnd,writeEnd;
		char* withoutSpaces = removeSpaces(token);
		char* executableName = getDescriptors(withoutSpaces,&readEnd,&writeEnd);
		writeEnd = pipe1[1];
		if(readEnd < 0)readEnd = 0;
		if(DEBUG){
			red();
			printf("\t\tStart: writeEnd - %d,readEnd - %d,executable - *%s*\n",writeEnd,readEnd,executableName);
			reset();
		}


		execute(executableName,readEnd,writeEnd);

		//THIS BLOCK EXECUTES MIDDLE
		for(int i = 0;i<pipes-1;i++){

			token = strtok(NULL,"|");
			withoutSpaces = removeSpaces(token);
			if(DEBUG){
				red();
				printf("\t\ti: %d,withoutSpaces: *%s*\n",i,withoutSpaces);
				reset();
			}

			if(i%2 == 0){
				execute(withoutSpaces,pipe1[0],pipe2[1]);
			}
			else{
				execute(withoutSpaces,pipe2[0],pipe1[1]);
			}
		}

		//THIS BLOCK EXECUTES END
		token = strtok(NULL,"|");
		withoutSpaces = removeSpaces(token);
		
		executableName = getDescriptors(withoutSpaces,&readEnd,&writeEnd);
		if(writeEnd < 0)writeEnd = 1;

		if(DEBUG){
			red();
			printf("\t\texecutableName: *%s*,readEnd - %d,writeEnd - %d\n",executableName,readEnd,writeEnd);
			reset();
		}
		if(pipes%2==0){
			close(pipe2[1]);
			execute(executableName,pipe2[0],writeEnd);
		}
		else{
			close(pipe1[1]);
			execute(executableName,pipe1[0],writeEnd);
		}
	}
}



int runCommand(char* string){
	if(DEBUG){
		red();
		printf("\tExecuting runCommand Function\n");
		reset();
	}
	char* command = strdup(string);
	char* token = strtok(command," ");

	if(strcmp(token,"cd") == 0){
		token = strtok(NULL," ");

		if(DEBUG){
			red();
			printf("\t\tChange Directory Token, Dir: *%s*\n",token);
			reset();
		}

		changeDirectory(token);

	}
	else if(strcmp(token,"pwd") == 0){
		
		if(DEBUG){
			red();
			printf("\t\tPrint Current Directory\n");
			reset();
		}

		token = strtok(NULL," ");
		printDirectory();
	}
	else if(strcmp(token,"rmdir") == 0){
		
		token = strtok(NULL," ");

		if(DEBUG){
			red();
			printf("\t\tRemove Directory Token, Dir : *%s*\n",token);
			reset();
		}

		removeDirectory(token);
	}
	else if(strcmp(token,"mkdir") == 0){
		
		token = strtok(NULL," ");
		
		if(DEBUG){
			red();
			printf("\t\tMake Directory Token, Dir : *%s*\n",token);
			reset();
		}

		makeDirectory(token);
	}
	else if(strcmp(token,"exit") == 0){

		if(DEBUG){
			red();
			printf("\t\tExit Token\n");
			reset();
		}
		
		return 0;
	}
	else{
		if(DEBUG){
			red();
			printf("\t\tUnkown Token\n");
			reset();
		}


	
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

		if(DEBUG){
			red();
			printf("\tReceived Command : *%s*\n",command);
			reset();
		}
		running = runCommand(command);
	}
}
/*

	start | middle | end

	start may have an input file  but wont have an output

	middle will neither have an input nor an output

	end might have output file but no input


	middle = number of pipes - 1

	start sends the output to pipe1

	middle[1] reads input from pipe1

	and writes it to pipe2

	middle[2] reads input from pipe2
	and writes it to pipe1

*/