/**
 * input.c contains functions that handle the user input from the terminal 
 * and parses the commands.
 * Author: Samantha Williams 
 * October 2, 2019
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "input.h"

/**
 * Returns the line of input the user enters from the terminal
 **/
char *get_line() {
    char *line = NULL;
    size_t size = 0;
    getline(&line, &size, stdin);
    return line;
}

/**
 * Inputs a char pointer and returns a pointer to char pointers.  Iterates through the input and separates the commands.
 **/
char **get_commands(char *in) {
    //allow a length of 64 for commands
    char **commands = malloc(256 * sizeof(char*));
    char *command;
    int idx = 0;
    command = strtok(in, ";\n");
    while (command != NULL) {
        commands[idx] = command;
        idx++;
        command = strtok(NULL, ";\n");
    }
    commands[idx] = NULL;
	free(command);
    return commands;
}

/**
 * Inputs an internal command and executes it without forking
 **/
int internal_commands(char *command, char *optionals) {
    char *internal[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "pwd"};
    int num_internal_commands = 8;
    int i;
    //cd
    if (strcmp(command, internal[0]) == 0) {
        if (optionals == NULL) {
            size_t BUFF_SIZE = 256 * sizeof(char);
	        char *buffer = malloc(BUFF_SIZE);
            getcwd(buffer, BUFF_SIZE);
            printf("%s\n", buffer);
			free(buffer);
        } else if (chdir(optionals) < 0) {
            printf("Error: Please enter a valid directory\n");
        }
    //clr
    } else if (strcmp(command, internal[1]) == 0) {
        printf("\033[H\033[J");
    //dir
    } else if (strcmp(command, internal[2]) == 0) {
		if (optionals == NULL) {
			system("ls");
			return 0;
		}
		size_t BUFF_SIZE = 256 * sizeof(char);
	    char *orig = malloc(BUFF_SIZE);
        getcwd(orig, BUFF_SIZE);
		if (chdir(optionals) < 0) {
			printf("Error: Cannot access directory\n");
			return 0;
		}
		system("ls");
		chdir(orig);
		free(orig);
    //environ
    } else if (strcmp(command, internal[3]) == 0) {
		system("env");
    //echo
    } else if (strcmp(command, internal[4]) == 0) {
		if (optionals == NULL) {
			printf("\n");
		} else {
			printf("%s\n", optionals);
		}
    //help
    } else if (strcmp(command, internal[5]) == 0) {
        printf("Samantha Williams's Shell\n");
        printf("The following commands are internal:\n");
        for (i=0; i<num_internal_commands; i++) {
            printf("%s\n", internal[i]);
        }
    //pause
    } else if (strcmp(command, internal[6]) == 0) {
        int ch = getchar();
        while (ch != 13 && ch != 10) {;}
    //pwd
    } else if (strcmp(command, internal[7]) == 0) {
        size_t BUFF_SIZE = 256 * sizeof(char);
	    char *buffer = malloc(BUFF_SIZE);
        getcwd(buffer, BUFF_SIZE);
        printf("%s\n", buffer);
		free(buffer);
    }
    return 0;
}

/** 
 * Inputs a char pointer and returns a 1 and executes the command if it is an internal command. If not, it returns 0.
 **/
int is_internal(char *command, char *optionals) {
    //list of internal commands
    char *internal[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "pwd"};
    int num_internal_commands = 8;
    int i;
    //check if the command matches one of the internal commands
    for (i=0; i<num_internal_commands; i++) {
        if ((strcmp(command, internal[i])==0)) {
            if(internal_commands(command, optionals) < 0) {
                printf("Error: not a valid internal command");
            }
            return 1;
        }
    }
    return 0;
}

/**
 * Inputs an external command and executes it utilizing forking
 **/
int fork_commands(char *command, char *optionals, int amp) {
    pid_t pid;
	int i = 1;
	//separate the arguments for the execvp call
	char *arguments[128 * sizeof(char*)];
	arguments[0] = command;
	char *arg = strtok(optionals, " \t\n");
	while (arg != NULL) {
		arguments[i] = arg;
		arg = strtok(NULL, " \t\n");
		i++;
	}
	arguments[i] = (char*) NULL;
		pid = fork();
		//child process
		if (pid == 0) {
			execvp(arguments[0], arguments);
			perror("execvp failed");
		} else {
			//return to the parent process if there is an ampersand present, otherwise, wait for the child process to finish running
			if (amp == 1) {
				fflush(stdout);
			} else {
				wait(NULL);
			}
	}
    return 0;
}

/**
 * Takes the file name as input, parses the file, and runs the commands within.
 **/
int file_handler(char *filename) {
	if (filename == NULL) {
		printf("Please enter a file name\n");
		return 0;
	}
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		perror("Filename");
		return 0;
	}
	char *input = NULL;
	size_t len;
	getdelim(&input, &len, '\0', f);
	char **list;
	fclose(f);
	list = get_commands(input);
	execute(list);
	return 0;
}

/**
 * Handles the case in which a pipe is being used to display the output
 **/
int pipe_handler(char *command, char *optionals) {
	int p[2];
	pipe(p);
	int i = 1;
	//separate the arguments for the execvp call
	char *arguments[128 * sizeof(char*)];
	arguments[0] = command;
	char *arg = strtok(optionals, " \t\n");
	while (strcmp(arg, "|") != 0) {
		arguments[i] = arg;
		arg = strtok(NULL, " \t\n");
		i++;
		printf("%s", arguments[i]);
	}
	arguments[i] = (char*) NULL;
	if (fork() == 0) {
		close(STDOUT_FILENO);
		dup(p[1]);
		close(p[0]);
		close(p[1]);
		execvp(arguments[0], arguments);
	}
	if (fork() == 0) {
		close(STDIN_FILENO);
		dup(p[0]);
		close(p[0]);
		close(p[1]);
		char *m[] = {"more", 0};
		execvp(m[0], m);
	}
	close(p[0]);
	close(p[1]);
	wait(NULL);
	wait(NULL);
	return 0;
}

/**
 * Takes in the command and arguments, parses, and executes the commands while printing output to output file
 **/
int redirect_output(char *command, char *optionals) {
	int i = 1;
	int append = 0;
	char *copy = malloc(sizeof(optionals));
	copy = optionals;
	//separate the arguments for the execvp call
	char *arguments[128 * sizeof(char*)];
	arguments[0] = command;
	char *arg = strtok_r(copy, " \t\n", &copy);
	while (strcmp(arg, ">") != 0 && strcmp(arg, ">>") != 0) {
		arguments[i] = arg;
		arg = strtok_r(copy, " \t\n", &copy);
		i++;
		if (strcmp(arg, ">>") == 0) {
			append = 1;
		}
	}	
	arguments[i] = (char*) NULL;
	char *filename = strtok_r(copy, "\t\n", &copy);
	if (fork() == 0) {
		int f;
		if (append) {
			f = open(filename, O_WRONLY | O_CREAT | O_APPEND);
		} else {
			f = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
		}		
		dup2(f, STDOUT_FILENO);
		close(f);
		if (is_internal(arguments[0], arguments[1])) {			
		} else {
			execvp(arguments[0], arguments);
		}
	}
	return 0;
}

/** 
 * Executes the given list of commands
 **/
int execute(char **list) {
    int i = 0;
	int amp = 0;
	int handled;
	int amp_char = '&';
	int pipe_char = '|';
	int redirect = '>';
    while (list[i] != NULL) {
		handled = 0;
        char *copy = malloc(sizeof(list[i]));
		strcpy(copy, list[i]);
         //separate first command in case there is an optional argument following it
        char *command = strtok_r(copy, " \n\0", &copy);
		char *optionals = strtok_r(copy, "\n\0", &copy);
		if (optionals != NULL) {
			if (strchr(optionals, amp_char) != NULL) {
				optionals = strtok(optionals, "&");
				amp = 1;
			}
			if (strchr(optionals, pipe_char) != NULL) {
				pipe_handler(command, optionals);
				//set handled to 1 to let the other statements know the commands have been handled
				handled = 1;
			}
			if (strchr(optionals, redirect) != NULL) {
				redirect_output(command, optionals);
				handled = 1;
			}
		}
        if(is_internal(command, optionals) && !handled) {
			
        } else if (strcmp(command, "myshell") == 0 && !handled) {
			file_handler(optionals);
			return 2;
		} else if (!handled){
            fork_commands(command, optionals, amp);
        }
        i++;
    }
    return 1;
}