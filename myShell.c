#include <stdio.h>       /* Input/Output */
#include <stdlib.h>      /* General Utilities */
#include <unistd.h>      /* Symbolic Constants */
#include <sys/types.h>   /* Primitive System Data Types */
#include <sys/wait.h>    /* Wait for Process Termination */
#include <errno.h>       /* Errors */
#include <string.h>      /* String Library */

char **strtoarray(char *line) {
    int i;
    char **array = malloc(sizeof(char*)*500);
    char *word = strtok(line, " \n");
    //printf("%s\n", word);
    
    for (i = 0; word!=NULL; i++) {
        array[i] = word;
        word = strtok(NULL, " \n");
        printf("%s\n", word);
    }
    array[i] = NULL;

    return array;
}

int main(int argc, char **argv) {
    int exitcmd = 0;
    char cmdLine[1000];
    char *cmdLnArg;
    char *cmdLnArg2;
    pid_t pid;
    char **array;
    int status;
    int i;
    char* test[2]={"ls", "-l"};

    while (exitcmd == 0) {
        printf(">");
        fgets(cmdLine, 1000, stdin);

        //checking if there are too many character in the command line
        if (strlen(cmdLine) == 999 && cmdLine[998] != '\n') {
            while (fgets(cmdLine, 1000, stdin) != NULL) {   
            }
            printf("myShell: too many characters\n");

        } else {//if valid no. of arguments, run rest of teh program
            cmdLine[strlen(cmdLine)-1] = '\0';//remove the \n character
            cmdLnArg = strtok(cmdLine, " ");//get the first arg from the argline

            if (strcmp(cmdLnArg, "exit")==0) { //user enters the exit command
                exitcmd = 1;
                printf("myShell terminating...\n\n");
                
            } else if (strcmp(cmdLnArg, "ls")==0) {//user enters ls command

                pid = fork();
                if(pid >= 0) {
                    if (pid == 0) {//child process
                        array = strtoarray(cmdLine);
                        printf("test\n");
                        status = execvp("ls", test);
                        printf("test\n");
                        
                        
                        free(array);
                        exit(status);
                    } else if(pid > 0) {//parent process
                        waitpid(pid, &status, 0);
                        if (WEXITSTATUS(status) == -1){
                            perror("ls");
                        }
                    }
                } else {
                    perror("fork");
                }
            } else {
                printf("myShell: %s: command not found\n", cmdLine);
            }
            
        }
    }
    return 0;
}