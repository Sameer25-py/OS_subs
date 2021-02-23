/**
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#define MAX_LINE		80 /* 80 chars per line, per command */

void get_input(char *args){
		int  i=0;
		while(true){
			char c = getchar();
			if(c == '\n') {
				args[i] = '\0';
				break;
			}
			else{
				args[i] = c;
				i++;
			}
			
		}
}
void copy_to_from_history(char input[],char history[]){
	if(input[0] == '!' && input[1] == '!' && input[2] == '\0') return;
	int  i = 0;
	while(input[i]!= '\0'){
		history[i] = input[i];
		i++;
	}
	history[i] = '\0';
}
void parse_input(char *input,char **tokens,char *history,bool flags[]){
	copy_to_from_history(input,history);
	char *token;
	int i = 0;
	token = strtok(input," \t\r\n\f\v\0");
	while (token != NULL ){
		if (strcmp(token,"&") == 0)flags[0] = true; //wait flag
		else if(strcmp(token,">")==0)flags[1] = true; //i/o redirect flag
		else if(strcmp(token,"exit")==0)flags[3] = true; //exit flag
		else if (strcmp(token,"|") == 0){flags[2] = true; tokens[i] = token; i++;} // pipe flag
		else {tokens[i] = token; i++;}
		token = strtok(NULL," \t\r\n\f\v\0");
	}
	tokens[i] = NULL;
	
}

void clear_flags(bool flags[]){
	int i = 0;
	while(i<4){
		flags[i] = false;
		i++;
	}
}
void command_splitter(char **tokens,char **command1, char **command2){
	int size = 0;
	int x1 = 0;
	int counter = 0;

	//size of tokens arr
	while(tokens[size] != NULL)size++;
	//positon of pipe
	while(strcmp(tokens[x1],"|") != 0)x1++;

	//copying before pipe
	for(int i=0;i<x1;i++){
		command1[counter] = tokens[i];
		counter+=1;
	}
	command1[counter]= NULL;
	counter = 0;
	//copying after pipe
	for(int i=x1+1;i<size;i++){
		command2[counter] = tokens[i];
		counter+=1;
	}
	command2[counter] = NULL;

}
void pipe_handler(char **tokens){
	char *command1[MAX_LINE/2 + 1];
	char *command2[MAX_LINE/2 + 1];
	int pp[2];
	int pp_stat=0;
	pid_t pid1,pid2;
	//spltting arguments in two arrays
	command_splitter(tokens,command1,command2);
	//creating pipes
	if(pipe(pp) == -1){
		cout << "ERROR CREATING PIPES " <<endl;
		return;
	}
	pid1 = fork();
	if(pid1 == 0){
		//child 1
		close(pp[0]);
		dup2(pp[1],STDOUT_FILENO);
		int status = execvp(command1[0],command1);
		if(status < 0)cout << "ERROR EXECUTING UNKNOWN COMMAND " << command1[0] <<endl;
		close(pp[1]);
		return;
	}
	pid2= fork();
	if(pid2 == 0){
		//child 2
		close(pp[1]);	
		dup2(pp[0],STDIN_FILENO);
		int status = execvp(command2[0],command2);
		if(status < 0)cout << "ERROR EXECUTING UNKNOWN COMMAND " << command2[0] <<endl;
		close(pp[0]);
		return;
	}
	//closing pipes in the parent space
	close(pp[0]);close(pp[1]);
	int status;
	//waiting on child processess to exit
	waitpid(pid1,&status,0);waitpid(pid2,&status,0); 
	return;


}
int i_o_redir_handler(char **tokens){
	//get filename
	int i =0;
	while(tokens[i]!=NULL){
		i++;
	}
	string filename = tokens[i-1];
	//get file descriptior for filename
	int fd = open(tokens[i-1],O_WRONLY | O_CREAT);
	if (fd < 0)
	{
		cout << "ERROR OPENING FILE " << filename <<endl;
		close (fd);
		return -1;
	}
	//getting copy of fd
	dup2(fd,1);

	
	return fd;
}

int execute_commands(char **args,bool flags[]){
	
	if (flags[2]){
		//pipe flag
		pipe_handler(args);
		return 0;
	}

	//fork
	pid_t pid;
	pid = fork();
	int status2;
	int status3;
	int fd = 0;
	if(pid == 0){
		//child process
			if (flags[1]){
				//replace stdout with outfile
				fd = i_o_redir_handler(args);
				if(fd == -1) { close(fd); return 0;}
			}
			int status = execvp(args[0],args);
			//closing fd
			if(fd){
				close(fd);
			}
			if(status < 0)cout << "ERROR EXECUTING UNKNOWN COMMAND " << args[0] <<endl;
	}

	//parent process
	if(!flags[0])waitpid(pid,&status2,0); //waiting on child process to exit	
	return 0;
}

int history_handler(char *input,char *history){
		if(input[0] == '!' && input[1] == '!' && input[2] == '\0'){
			//history flag
		if(history[0] == ' '){
			//empty history
			cout << "NO COMMANDS IN HISTORY" <<endl;
			return -1; 
		}
		else{
			copy_to_from_history(history,input);
		}
	}
	return 0;

}


int main(void)
{		char  input[MAX_LINE]; 
		char *args[MAX_LINE/2 + 1];
		char history[MAX_LINE/2 + 1]; /* command line (of 80) has max of 40 arguments */						
		bool flags[4]={false,false,false,false};  //wait flag,i/o <-> flag,pipe flag,exit flag
    	int should_run = 1;
		history[0] = ' '; //empty history initialized
    	while (should_run){ 
			char  input[MAX_LINE];   
			printf("osh>");
			fflush(stdout);
			get_input(input); //get input from stdin character by character
			if(history_handler(input,history) == -1) {clear_flags(flags);continue;}; // check if history flag is true. Replace input with the history else restart the loop.
			parse_input(input,args,history,flags); //convert input into tokens and check for flags. Also copy input to history before parsing.
			if(flags[3]){should_run = 0; continue;} //check for exit flag and exits out.
			execute_commands(args,flags); //execute args using execvp command. Checks for the redirection flag,pipe flag and behave accordingly.
			clear_flags(flags); //clear all the flags parsed from the input.
			/**
         	 * After reading user input, the steps are:
         	 * (1) fork a child process
         	 * (2) the child process will invoke execvp()
         	 * (3) if command includes &, parent and child will run concurrently
         	 */
    	}
    
	return 0;
}
