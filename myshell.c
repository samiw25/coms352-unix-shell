/**
 * myshell.c includes the main function, initializing the shell, and the basic input_handler
 * Author: Samantha Williams 
 * September 29, 2019
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "input.h"

//escape sequence to clear the terminal
#define clear() printf("\033[H\033[J")

/**
 * Initialize the shell by clearing the terminal and printing a welcome message
 **/
void init_shell() {
	clear();
	printf("WELCOME TO MY SHELL\n");
	sleep(1);
	clear();
}

/**
 * Calls the functions necessary to handle the input from the user
 * Returns 0 if the user quits the program
 **/
int input_handler() {
	char *input;
	char **commands;
	int flag = 1;
	size_t BUFF_SIZE = 256 * sizeof(char);
	char *buffer = malloc(BUFF_SIZE);
	while (flag == 1) {
		getcwd(buffer, BUFF_SIZE);
		printf("%s/myshell>: ", buffer);
		input = get_line();
		if (strcmp(input, "quit\n") == 0) {
			return 0;
		}
		commands = get_commands(input);
		flag = execute(commands);
	}
	free(buffer);
	return flag;
}

/**
 * Handles the commands when the user includes a filename argument after ./myshell
 **/
int file_input_handler(char *filename) {
	file_handler(filename);
}

/**
 * Run the program by initializing and calling the input handler to wait for user commands
 **/
int main(int argc, char** argv) {
	init_shell();
	if (argc > 1) {
		file_input_handler(argv[1]);
		return 0;
	}
	int result = input_handler();
	if (result == 0) {
		printf("You have quit the shell.\n");
		printf(" Have a great day!\n");
	} else if (result == -1) {
		printf("An error has occured.  The program will now end. \n");
	} 
	return 0;
}