#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAXLINE 124  // maximum characters in a line

void shell_loop()
{
    char command[MAXLINE];   // character array for command 
    char *args[128];          // Array to hold command arguments

    /* Define the shell loop */
    while (1)
    {
        // 1. Shell prompt
        printf("> ");
        
        // 2. Read user input
        fgets(command, MAXLINE, stdin);
        
        // 3. Parse the input.
        args[0] = strtok(command, " \n");
        int i = 0;
        
        while (args[i] != NULL) {
            i++;
            args[i] = strtok(NULL, " \n");
        }
        
        // Handling built-in commands
        if (strcmp(args[0], "exit") == 0)   // exit
            break;
        else if (strcmp(args[0], "echo") == 0) {  // echo
	    for (int j = 1; j < i; j++)
		printf("%s ", args[j]);
	}
        else if (strcmp(args[0], "pwd") == 0) {
	    char cwd[256];  // buffer
	    getcwd(cwd, sizeof(cwd));   
	}
	else if (strcmp(args[0], "cd") == 0) {
	    if (strcmp(args[1], "~") == 0) { 
		chdir(getenv("HOME"));
            }
	    else if (strcmp(args[1], "/") == 0) {
		chdir("/");
	    }
	    else
	        chdir(args[1]);
        }

        // Handling redirection
        int in_redirect = 0;
        int out_redirect = 0;
        int append = 0;
        char *filepath = NULL;
        
        // Parsing the command to check for redirection (input redirection, output redirection, and append)
        for (int j = 0; j < i; j++) {
	    if (strcmp(args[j], "<") == 0) {
		in_redirect = 1;
		filepath = args[j+1];
		args[j] = NULL;
	
	    } else if (strcmp(args[j], ">") == 0) {
		out_redirect = 1;
		filepath = args[j+1];
		args[j] = NULL;
	    } else if (strcmp(args[j], ">>") == 0) {
		in_redirect = 1;
		append = 1;
		filepath = args[j+1];
		args[j] = NULL;
            }
	}
        
        // Create a new process
        pid_t pid = fork();    // Create a new process
       
        if (pid == 0) {
	    int fd;
	    if (out_redirect) {  /* Point the stdin descriptor to the file for redirection */
		if (append == 1) 
		    fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
	        else 
		    fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		dup2(fd, STDOUT_FILENO);

	    } else if (in_redirect) { 
		fd = open(filepath, O_RDONLY);
		dup2(fd, STDIN_FILENO);
	    }

	    close(fd);

	    execvp(args[0], args);

	    // If execvp() fails, we reach here:
	    perror("execvp failed");
	    exit(EXIT_FAILURE);	
	} else if (pid == -1) {
	    perror("fork failed");
        } else
	    wait(NULL);
        printf("\n");
    }

}

int main()
{
    shell_loop();
    return 0;
}

