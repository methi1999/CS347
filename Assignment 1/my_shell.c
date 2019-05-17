//Make cd a child process for uniformity
#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define SIGQUIT 3

int pgid_to_kill = -1;
int bg_pid_to_kill = -2;
int saadha_kill = -3;
int current_bg_processes[MAX_NUM_TOKENS];
int current_bg_num = 0;
/* Splits the string by space and returns the array of tokens
*
*/
void kill_pg(int x){

	if (pgid_to_kill != -1){
		// printf("Killing %i\n", pgid_to_kill);
		int ret = killpg(pgid_to_kill, SIGKILL);
		// printf("%i", ret);
		pgid_to_kill = -1;
	}
}

void child_handler(int x){
	// printf("In child handler\n");
	int check_bg = waitpid(-1, NULL, WNOHANG);
		// printf("Check bg %i\n", check_bg);
	if (check_bg > 0){
		printf("Shell: Background process finished\n");
		// printf("Killed %i\n", check_bg);
	}
}

void kill_saadha(int num){
	if (saadha_kill != -3){
		// printf("Killing %i\n", saadha_kill);
		int ret = kill(saadha_kill, SIGKILL);
		saadha_kill = -3;
	}
}

// void kill_pg_back(int x){

// 	if (bg_pid_to_kill != -2){

// 		int ret = kill(bg_pid_to_kill, SIGKILL);
// 		bg_pid_to_kill = -2;
// 	}
// }

void sig_ignore(int x){
	// printf("\n");
}

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

	char readChar = line[i];

	if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
	  token[tokenIndex] = '\0';
	if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
	  }
	} else {
	  token[tokenIndex++] = readChar;
	}
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {

	signal(SIGINT, sig_ignore);
	char  line[MAX_INPUT_SIZE];
	char  **tokens;			  
	int i;

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		// printf("Beginning input\n");
		//Zero byte a string
		signal(SIGINT, sig_ignore);
		child_handler(0);
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			//fgets - store lines in line variable from fp
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		/* END: TAKING INPUT */
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
		
		// exec
		// printf("main: %i, main_parent: %i, main_group: %i\n", getpid(), getppid(), getpgrp());

		int tokens_length = 0;
		while (tokens[tokens_length] != NULL)
			tokens_length++;

		if (tokens_length == 0)
			continue;

		if (!strcmp(tokens[0],"exit"))
		{
			if (current_bg_num > 0)
			{
				for (int i = 0; i < current_bg_num; ++i)
				{
					kill(current_bg_processes[i], SIGKILL);
					// printf("Killing\n");
				}
				// kill(bg_pid_to_kill, SIGKILL);
			}
			printf("Shell: Goodbye.\n");
			exit(0);
		}
		
		int single_amp = 0, double_amp = 0, triple_amp = 0;
		for (int i = 0; i < tokens_length; ++i)
		{
			if (!strcmp(tokens[i], "&"))
				single_amp++;
			else if (!strcmp(tokens[i], "&&"))
				double_amp++;
			else if (!strcmp(tokens[i], "&&&"))
				triple_amp++;
		}
	   //*****single amp - background********

		if (single_amp != 0){


			int check = strcmp(tokens[0], "cd");
			if (check == 0) {
				// printf("Changing working directory to %s\n",tokens[1]);
				int try = chdir(tokens[1]);
				// printf("%i",try);
				if (try == -1)
					printf("Shell: Incorrect command\n");
				else
					// printf("continue");
					continue;
			}

				// printf("ret: %i, ret_parent: %i, ret_group: %i\n", getpid(), getppid(), getpgrp());
			int ret = fork();

			if (ret == 0){

				setpgrp();
				tokens[tokens_length-1] = NULL;
				int try_1 = execv(tokens[0], tokens);
				// printf("%i\n", try_1);
				if (try_1 == -1) {			
					char * new_str ;
					char * base_path = "/bin/";
					if((new_str = malloc(strlen(tokens[0])+strlen(base_path)+1)) != NULL){
						new_str[0] = '\0';   // ensures the memory is an empty string
						strcat(new_str,base_path);
						strcat(new_str,tokens[0]);
					} 
					else {
						// fprintf(STDERR,"malloc failed!\n");
						printf("Couldn't concatenate strings\n");
						// exit?
					}
					// printf("%s\n", new_str);

					int return_code = execv(new_str, tokens);
					if (return_code == -1)
						printf("Shell: Incorrect command\n");
					// printf("Returned %i\n", ret);	
				}
			}
			else {
				current_bg_processes[current_bg_num] = ret;
				current_bg_num++; 
			}
			
			// else - don't wait

		}

		//********double amp - multiple sequential foreground***********

		else if (double_amp != 0){

			// printf("In double amp\n");
			signal(SIGINT, kill_pg);
			int current_index = 0;
			
			int previous_index = 0;

			if (!strcmp(tokens[current_index], "&&"))
			{
				// printf("Running command from %s to %s\n", tokens[previous_index], tokens[current_index-1]);
				char *args[current_index-previous_index+1];
				args[current_index-previous_index] = NULL;
				for (int i = previous_index; i < current_index; ++i){
					args[i-previous_index] = tokens[i];
					// printf("Adding arg: %s\n", args[i-previous_index]);
				}
				previous_index = current_index+1;

				int check = strcmp(args[0], "cd");
				int to_cd = 0;
				if (check == 0)
				{
					// printf("Changing working directory to %s\n",tokens[1]);
					int try = chdir(args[1]);
					// printf("%i",try);
					if (try == -1)
						printf("Shell: Incorrect command\n");

					current_index++;
					to_cd = 1;
					
				}

				//Execute the command
				if (!to_cd)
				{
					int ret = fork();

					if (ret == 0){
						// printf("%s\n", tokens[0]);
						setpgrp();
						// printf("bg process group id: %i and parent: %i\n", getpgid(0), getppid());

						int try_1 = execv(tokens[0], args);
						
						if (try_1 == -1) {
							char * new_str ;
							char * base_path = "/bin/";
							if((new_str = malloc(strlen(args[0])+strlen(base_path)+1)) != NULL){
								new_str[0] = '\0';   // ensures the memory is an empty string
								strcat(new_str,base_path);
								strcat(new_str,args[0]);
							} 
							else {
								// fprintf(STDERR,"malloc failed!\n");
								printf("Couldn't concatenate strings\n");
								// exit?
							}
							// printf("Exec:%s\n", new_str);
							int ret = execv(new_str, args);
							if (ret == -1)
								printf("Shell: Incorrect command\n");
							// printf("Returned %i\n", ret);
						}
					}
				
					else
					{
						pgid_to_kill = ret;
						waitpid(ret, 0, 0);
					}
				}
				else
					pgid_to_kill = -1;
				
			}//Done with execution	

			current_index++;
			
			while (tokens[current_index] != NULL) {
			
				if (!strcmp(tokens[current_index], "&&"))
				{
					// printf("Running command from %s to %s\n", tokens[previous_index], tokens[current_index-1]);
					char *args[current_index-previous_index+1];
					args[current_index-previous_index] = NULL;
					for (int i = previous_index; i < current_index; ++i){
						args[i-previous_index] = tokens[i];
						// printf("Adding arg: %s\n", args[i-previous_index]);
					}
					previous_index = current_index+1;

					int check = strcmp(args[0], "cd");
					if (check == 0)
					{
						// printf("Changing working directory to %s\n",tokens[1]);
						int try = chdir(args[1]);
						// printf("%i",try);
						if (try == -1)
							printf("Shell: Incorrect command\n");

						current_index++;
						continue;
					}

					//Execute the command
					int ret = fork();

					if (ret == 0){
						// printf("%s\n", tokens[0]);
						if (pgid_to_kill == -1)
							setpgid(0, 0);
						else
							setpgid(ret, pgid_to_kill);
						// printf("%s pgid: %i\n", tokens[0], getpgid(0));

						int try_1 = execv(tokens[0], args);
						
						if (try_1 == -1) {
							char * new_str ;
							char * base_path = "/bin/";
							if((new_str = malloc(strlen(args[0])+strlen(base_path)+1)) != NULL){
								new_str[0] = '\0';   // ensures the memory is an empty string
								strcat(new_str,base_path);
								strcat(new_str,args[0]);
							} 
							else {
								// fprintf(STDERR,"malloc failed!\n");
								printf("Couldn't concatenate strings\n");
								// exit?
							}
							// printf("Exec:%s\n", new_str);
							int ret = execv(new_str, args);
							if (ret == -1)
								printf("Shell: Incorrect command\n");
							// printf("Returned %i\n", ret);
						}
					}
					else
					{
						waitpid(ret, 0, 0);
						// printf("Done with %s\n", args[0]);
						// printf("New start: %i\n", previous_index);
					}
				}//Done with execution	

				current_index++;
				// printf("%i\n", current_index);
			}

			if (tokens[previous_index] != NULL)
			{
				// printf("Last\n");
				// printf("Running command from %s to %s\n", tokens[previous_index], tokens[current_index-1]);
				char *args[current_index-previous_index+1];
				args[current_index-previous_index] = NULL;
				for (int i = previous_index; i < current_index; ++i){
					args[i-previous_index] = tokens[i];
					// printf("Adding arg: %s\n", args[i-previous_index]);
				}

				int check = strcmp(args[0], "cd");
				if (check == 0) {
					// printf("Changing working directory to %s\n",tokens[1]);
					int try = chdir(args[1]);
					// printf("%i",try);
					if (try == -1)
						printf("Shell: Incorrect command\n");
					continue;
				}				
				//Execute the command
				int ret = fork();

				if (ret == 0){

					setpgid(ret, pgid_to_kill);
					// printf("%s pgid: %i\n", tokens[0], getpgid(0));
					int try_1 = execv(args[0], args);
						
					if (try_1 == -1) {

						char * new_str ;
						char * base_path = "/bin/";
						if((new_str = malloc(strlen(args[0])+strlen(base_path)+1)) != NULL){
							new_str[0] = '\0';   // ensures the memory is an empty string
							strcat(new_str,base_path);
							strcat(new_str,args[0]);
						} 
						else 
						{
							// fprintf(STDERR,"malloc failed!\n");
							printf("Couldn't concatenate strings\n");
							// exit?
						}
						// printf("last %s\n", new_str);
						int ret = execv(new_str, args);
						if (ret == -1)
							printf("Shell: Incorrect command\n");
						// printf("Returned %i\n", ret);
					}
				}
				else 
				
					waitpid(ret, 0, 0);
				
			} // End last block fo input
		
		} // End double amp

		//********* triple-amp: multiple parralel foreground *******

		else if (triple_amp != 0)
		{

			int current_index = 0;
			int previous_index = 0;
			int valid = triple_amp+1;
			signal(SIGINT, kill_pg);

			while(strcmp(tokens[current_index], "&&&"))
				current_index++;

			// printf("Before: group id: %d\n", getpgrp());
			if (!strcmp(tokens[current_index], "&&&"))
			{
				// printf("Running command from %s to %s\n", tokens[previous_index], tokens[current_index-1]);
				char *args[current_index-previous_index+1];
				args[current_index-previous_index] = NULL;
				for (int i = previous_index; i < current_index; ++i){
					args[i-previous_index] = tokens[i];
					// printf("Adding arg: %s\n", args[i-previous_index]);
				}
				previous_index = current_index+1;

				int check = strcmp(args[0], "cd");
				if (check == 0) {
					valid--;
					// printf("Changing working directory to %s\n",tokens[1]);
					int try = chdir(args[1]);
					// printf("%i",try);
					if (try == -1)
						printf("Shell: Incorrect command\n");
					
					current_index++;
					// printf("Continuing\n");
					continue;
				}
				//Execute the command
				int ret = fork();

				if (ret == 0)
				{
					// printf("Try 1: %s\n", args[0]);
					setpgrp();
					int try_1 = execv(args[0], args);
					
					if (try_1 == -1) {
						// printf("Exec\n");
						// printf("%s : %d\n", args[0], getpgrp());
						char * new_str ;
						char * base_path = "/bin/";
						if((new_str = malloc(strlen(args[0])+strlen(base_path)+1)) != NULL){
							new_str[0] = '\0';   // ensures the memory is an empty string
							strcat(new_str,base_path);
							strcat(new_str,args[0]);
						} 
						else {
							// fprintf(STDERR,"malloc failed!\n");
							printf("Couldn't concatenate strings\n");
							// exit?
						}
						// printf("Exec:%s\n", new_str);
						int ret = execv(new_str, args);
						if (ret == -1){
							printf("Shell: Incorrect command\n");
							valid--;
						}
					}
				}
				else
					pgid_to_kill = ret;
				// else - No need to wait
			}

			current_index++;
			// printf("In wrapper\n");
			// printf("wrapper pid, pgid: %d, %d\n", getpid(), getpgrp());
			// signal(SIGINT, sig_ignore);
			while (tokens[current_index] != NULL)
			{
			
				if (!strcmp(tokens[current_index], "&&&"))
				{
					// printf("Running command from %s to %s\n", tokens[previous_index], tokens[current_index-1]);
					char *args[current_index-previous_index+1];
					args[current_index-previous_index] = NULL;
					for (int i = previous_index; i < current_index; ++i){
						args[i-previous_index] = tokens[i];
						// printf("Adding arg: %s\n", args[i-previous_index]);
					}
					previous_index = current_index+1;

					int check = strcmp(args[0], "cd");
					if (check == 0) {
						valid--;
						// printf("Changing working directory to %s\n",tokens[1]);
						int try = chdir(args[1]);
						// printf("%i",try);
						if (try == -1)
							printf("Shell: Incorrect command\n");
						
						current_index++;
						// printf("Continuing\n");
						continue;
					}
					//Execute the command
					int ret = fork();

					if (ret == 0)
					{
						// printf("Try 1: %s\n", args[0]);
						setpgid(ret, pgid_to_kill);
						int try_1 = execv(args[0], args);
						
						if (try_1 == -1) {
							// printf("Exec\n");
							// printf("%s : %d\n", args[0], getpgrp());
							char * new_str ;
							char * base_path = "/bin/";
							if((new_str = malloc(strlen(args[0])+strlen(base_path)+1)) != NULL){
								new_str[0] = '\0';   // ensures the memory is an empty string
								strcat(new_str,base_path);
								strcat(new_str,args[0]);
							} 
							else {
								// fprintf(STDERR,"malloc failed!\n");
								printf("Couldn't concatenate strings\n");
								// exit?
							}
							// printf("Exec:%s\n", new_str);
							int ret = execv(new_str, args);
							if (ret == -1){
								printf("Shell: Incorrect command\n");
								valid--;
							}
						}
					}
					// else - No need to wait
				}

				current_index++;
				// printf("%i\n", current_index);
			}
			// printf("Done with last but one double amp\n");
			int is_cd = 0;
			
			if (tokens[previous_index] != NULL)
			{
				// printf("Last\n");
				// printf("Running command from %s to %s\n", tokens[previous_index], tokens[current_index-1]);
				char *args[current_index-previous_index+1];
				args[current_index-previous_index] = NULL;

				for (int i = previous_index; i < current_index; ++i){
					args[i-previous_index] = tokens[i];
					// printf("Adding arg: %s\n", args[i-previous_index]);
				}// printf("%s\n", args[i-previous_index]);

				int check = strcmp(args[0], "cd");
				if (check == 0) {
					valid--;
					// printf("Changing working directory to %s\n",tokens[1]);
					int try = chdir(args[1]);
					// printf("%i",try);
					if (try == -1)
						printf("Shell: Incorrect command\n");
					is_cd = 1;
					// printf("Continuing\n");
				}
				if (is_cd == 0){
					//Execute the command
					// printf("Forking\n");
					int ret = fork();

					if (ret == 0){
						// printf("In ret\n");
						setpgid(ret, pgid_to_kill);
						int try_1 = execv(args[0], args);
						if (try_1 == -1) {
							// printf("%s : %d\n", command, getpgrp());
							// printf("HEre\n");
							char * new_str ;
							char * base_path = "/bin/";
							if((new_str = malloc(strlen(args[0])+strlen(base_path)+1)) != NULL){
								new_str[0] = '\0';   // ensures the memory is an empty string
								strcat(new_str,base_path);
								strcat(new_str,args[0]);
							} 
							else {
								// fprintf(STDERR,"malloc failed!\n");
								printf("Couldn't concatenate strings\n");
								// exit?
							}
							// printf("%s\n", new_str);
							int ret = execv(new_str, args);
							if (ret == -1){
								valid--;
								printf("Shell: Incorrect command\n");
							}
						}
					
					}
				} // Done with all
				// printf("Done with all\n");
				// No need for else
				// else {
				// 	wait(0);
				// }
				// 	waitpid(ret, 0, 0);
				// 	// printf("Done with %s\n", args[0]);
				// 	previous_index = current_index+1;
				// 	// printf("New start: %i\n", previous_index);
				// }
			}
			// printf("Valid: %i\n", valid);
			
			for (int i = 0; i < valid; ++i)
			{
				wait(0);
				// printf("Reaped\n");
			}
		
		}
		
		else {
			//*******Saaaadha
			// printf("Saadha\n");

			int check = strcmp(tokens[0], "cd");
			if (check == 0) {
				// printf("Changing working directory to %s\n",tokens[1]);
				int try = chdir(tokens[1]);
				// printf("%i",try);
				if (try == -1)
					printf("Shell: Incorrect command\n");
				continue;
			}

			signal(SIGINT, kill_saadha);

			int ret = fork();
			if (ret == 0){

				int try_1 = execv(tokens[0], tokens);
				if (try_1 == -1){
					char * new_str ;
					char * base_path = "/bin/";
					if((new_str = malloc(strlen(tokens[0])+strlen(base_path)+1)) != NULL){
						new_str[0] = '\0';   // ensures the memory is an empty string
						strcat(new_str,base_path);
						strcat(new_str,tokens[0]);
					} 
					else {
						// fprintf(STDERR,"malloc failed!\n");
						printf("Couldn't concatenate strings\n");
						// exit?
					}
					// printf("%s\n", new_str);
					int ret = execv(new_str, tokens);
					if (ret == -1)
						printf("Shell: Incorrect command\n");
					// printf("Returned %i\n", ret);
				}
			}
			else {
				saadha_kill = ret;
				wait(0);
			}
		}	   

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}

