/**
 * The header file for input.c
 * Author: Samantha Williams
 * October 3, 2016
 **/
#ifndef INPUT_H
#define INPUT_H

/**
 * Returns the line of input the user enters from the terminal
 **/
char *get_line();

/**
 * Inputs a char pointer and returns a pointer to char pointers.  Iterates through the input and separates the commands.
 **/
char **get_commands(char *in);

/**
 * Inputs an internal command and executes it without forking
 **/
int internal_commands(char *command, char *optionals);

/** 
 * Inputs a char pointer and returns 1 if the command is an internal command and 0 if it is not.
 **/
int is_internal(char *command, char *optionals);

/**
 * Inputs an external command and executes it utilizing forking
 **/
int fork_commands(char *command, char *optionals, int amp);

/**
 * Takes the file name as input, parses the file, and runs the commands within.
 **/
int file_handler(char *filename);

/**
 * Handles the case in which a pipe is being used to display the output
 **/
int pipe_handler(char *command, char *optionals);

/**
 * Takes in the command and arguments, parses, and executes the commands while printing output to output file
 **/
int redirect_output(char *command, char *optionals);

/** 
 * Executes the given list of commands
 **/
int execute(char **list);

#endif