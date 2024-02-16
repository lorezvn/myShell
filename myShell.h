#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFFER_SIZE 512


//Prototipi
void execute_command(char ** argv);
void change_directory(char ** argv);
void get_command(char * command, char **argv); 