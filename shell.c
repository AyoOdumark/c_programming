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
            continue;
	}
        else if (strcmp(args[0], "pwd") == 0) {
	    char cwd[256];  // buffer
	    getcwd(cwd, sizeof(cwd));
	    printf("%s", cwd);
	    continue;   
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
	    continue;
        }

        // Handling redirection and pipe
        int in_redirect = 0;
        int out_redirect = 0;
        int append = 0;
	int isPipe = 0;
	int splitIndex = -1;
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
            } else if (strcmp(args[j], "|") == 0) {
		splitIndex = j;
		isPipe = 1;
	    }
	}

        // If it is a pipe command then create two processes
    	if (isPipe) {
	    pid_t pid1, pid2;
            int pipefd[2];
            	  
	    int firstSize = splitIndex + 1;
            int secondSize = (i - splitIndex);

            char *firstArgs[firstSize];
            char *secondArgs[secondSize];
                
            for (int j = 0; j < firstSize; j++) {
                firstArgs[j] = args[j];
		if (strcmp(args[j], "|") == 0)
		    firstArgs[j] = NULL;
            }
            for (int j = 0; j < secondSize; j++)
                secondArgs[j] = args[splitIndex + j + 1];
            
	    secondArgs[secondSize] = NULL;

	    if (pipe(pipefd) == -1) {
                perror("pipe failed");
                exit(EXIT_FAILURE);
            }

	    pid1 = fork();
            if (pid1 == 0) {
		
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                execvp(firstArgs[0], firstArgs);

                perror("execvp failed");
		exit(EXIT_FAILURE);
	    } else if (pid1 == -1) {
		perror("fork failure");
		exit(EXIT_FAILURE);
	    }

	    pid2 = fork(); 
	    if (pid2 == 0) {
		
                close(pipefd[1]);
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[0]);

		execvp(secondArgs[0], secondArgs);

                perror("execvp failed");
		exit(EXIT_FAILURE);
	    } else if (pid2 == -1) {
		perror("fork failure");
		exit(EXIT_FAILURE);
            }

	    close(pipefd[0]);
	    close(pipefd[1]);
	    waitpid(pid1, NULL, 0);
	    waitpid(pid2, NULL, 0);
        } else {

            // Create a new process
            pid_t pid = fork();    // Create a new process
       
            if (pid == 0) {
	        if (out_redirect) {  /* Point the stdin descriptor to the file for redirection */
		    int fd;
		    if (append == 1) 
		        fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
	            else 
		        fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		    dup2(fd, STDOUT_FILENO);
                    close(fd);
	        } else if (in_redirect) { 
		    int fd = open(filepath, O_RDONLY);
		    dup2(fd, STDIN_FILENO);
                    close(fd);
	        }

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
}

int main()
{
    shell_loop();
    return 0;
}

