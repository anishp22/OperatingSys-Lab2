#include<sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<fcntl.h>
#include <dirent.h>

#include "utility.h"
#include "myshell.h"

// Macros / constants with #define
#define BUFFER_LEN 256
#define SCREENBUFFER 100

// global variables below
int pid;
char shell[BUFFER_LEN];
const char *prompt = "$";
char pwd[BUFFER_LEN];


// opens the README file and displays it to user
void showHelp() {
	FILE *f = fopen("README.md", "r");
	char row[255];
	while (fgets(row, sizeof(row), f) != NULL)
		printf("%s", row);
	fclose(f);
}

int main(int argc, char *argv[]) {
	// Buffer
	char arg[BUFFER_LEN] = { 0 };
	char* token;
	char command[BUFFER_LEN] = { 0 };
	char buffer[BUFFER_LEN] = { 0 };
	
	FILE *batchfile;// batchfile pointer

	getcwd(pwd, sizeof(pwd));// set pwd

	pid = getpid();// get PID

	// shell to path /prov/2636/exe
	char linkpath[50];
	sprintf(linkpath, "/proc/%d/exe", pid);
	readlink(linkpath, shell, BUFFER_LEN);

	// batchfile pointer
	if (argc > 1) {
		if ((batchfile = fopen(argv[1], "r")) == 0) {
			perror(argv[1]);
			return EXIT_SUCCESS;
		}
	}

	// get user input
	// infinite loop until user quits
	while (1) {

		// reset buffers
		buffer[0] = '\0';
		command[0] = '\0';
		arg[0] = '\0';

		//prompt
		printf("%s%s ", pwd, prompt);

		// read batchfile
		if (argc > 1) {
			//exit if invalid
			if (fgets(buffer, BUFFER_LEN, batchfile) == NULL)
				strcpy(buffer, "exit");

			//valid
			else {
				//empty line
				if (strlen(buffer) <= 1)
					continue;

				// display and run command
				printf("Running: %s\n", buffer);
			}
		}
		
		//exit if input null
		else if (fgets(buffer, BUFFER_LEN, stdin) == NULL) {
			printf("Exit shell\n");
			return EXIT_SUCCESS;
		}
		
		// remove newline char
		token = strtok(buffer, "\n");
		
		// get input
		token = strtok(buffer, " ");
		strcpy(command, token);


		// get arguments by tokenizing
		token = strtok(NULL, " ");
		if (token != NULL) {
			strcpy(arg, token);
			while (1) {
				token = strtok(NULL, " ");
				if (token == NULL)
					break;
				strcat(arg, " ");
				strcat(arg, token);
			}
		}

		// help command 
		if (strcmp(command, "help") == 0) {
			showHelp();
		}
		
		// environ command
		else if (strcmp(command, "environ") == 0) {
			printf(" PID: %d\n SHELL: %s\n PWD: %s\n", pid, shell, pwd);
		}		
		
		// cd command
		else if (strcmp(command, "cd") == 0) {

			//no path given
			if (strlen(arg) == 0)
				strcpy(arg, pwd);

			// invalid dir
			if (chdir(arg) != 0)
				perror("sh");

			// set pwd
			else
				getcwd(pwd, sizeof(pwd));
		}
		
		// dir command
		else if (strcmp(command, "dir") == 0) {
			DIR *dir;
			struct dirent *dp;

			// pwd as <arg> if no path given
			if (strlen(arg) == 0)
				strcpy(arg, pwd);

			// dir with path <arg>
			dir = opendir(arg);

			// invalid dir
			if (dir == NULL)
				perror(arg);

			else {
				// retrieve and print all dir contents
				while((dp = readdir(dir)) != NULL)
					printf("%s\t", dp->d_name);
				printf("\n");
			}

			// close dir
			closedir(dir);
		}

		// pause command
		else if (strcmp(command, "pause") == 0) {
			char input[255];
			printf("%s\n", "Paused: Press Enter to resume");
			do {
				fgets(input, sizeof(input), stdin);
			} while(input[0] != '\n');
		}

		// echo command
		else if (strcmp(command, "echo") == 0) {
			// if no <comment> is specified, set arg to empty string
			if (strlen(arg) == 0)
				strcpy(arg, "");
			printf("%s\n", arg);
		}

		// clr command
		else if (strcmp(command, "clr") == 0 || strcmp(command, "clear") == 0) {
			int i;
			for(i=0; i<SCREENBUFFER; i++) printf("\n");
		}
		

		// exit command
		else if (strcmp(command, "quit") == 0 || (strcmp(command, "exit")) == 0) {
			printf("Exit shell\n");
			return EXIT_SUCCESS;
		}

		// invalid input
		else {
			fputs("Invalid command. Use 'help' for assistance. \n", stderr);
		}
	}

	// close batchfile
	if (argc > 1)
		fclose(batchfile);

	return EXIT_SUCCESS;
}
