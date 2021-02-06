#include <stdio.h>       /* Input/Output */
#include <stdlib.h>      /* General Utilities */
#include <unistd.h>      /* Symbolic Constants */
#include <sys/types.h>   /* Primitive System Data Types */
#include <sys/wait.h>    /* Wait for Process Termination */
#include <errno.h>       /* Errors */
#include <string.h>      /* String Library */
#include <signal.h>      /* Signal Library */

char **strtoarray(char *line) {
    int i;
    char **array = malloc(sizeof(char*)*500);
    char *word = strtok(line, " \n");
    
    for (i = 0; word!=NULL; i++) {
        array[i] = word;
        word = strtok(NULL, " \n");
    }
    array[i] = NULL;
    return array;
}

void sigquit(int signo) {
    printf(">");
    exit(0);
}

int main(int argc, char **argv) {
    int exitcmd = 0;
    char cmdLine[1000], cmdLineTokens[1000];
    char *cmdLnArg;
    pid_t pid;
    pid_t asynchpid;
    char **array;
    int status;
    
    struct sigaction sigact;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);

    sigact.sa_handler = sigquit;
    if (sigaction(SIGQUIT, &sigact, NULL) < 0) {
        perror("sigaction()");
        exit(1);
    }

    while (exitcmd == 0) {//loop until exit is enterred
        fgets(cmdLine, 1000, stdin);

        //checking if there are too many character in the command line
        if (strlen(cmdLine) == 999 && cmdLine[998] != '\n') {
            while (fgets(cmdLine, 1000, stdin) != NULL) {   
            }
            printf("myShell: too many characters\n");

        } else {//if valid no. of arguments, run rest of teh program
            cmdLine[strlen(cmdLine)-1] = '\0';//remove the \n character
            strcpy(cmdLineTokens, cmdLine);

            //to run command options
            cmdLnArg = strtok(cmdLineTokens, " ");//get the first arg from the argline

            if (strcmp(cmdLnArg, "exit")==0) {//exit command
                exitcmd = 1;
                printf("myShell terminating...\n\n");
                kill(asynchpid,SIGQUIT);
                
            } else if (strcmp(cmdLnArg, "ls")==0) {//ls command

                pid = fork();
                if(pid >= 0) {//if valid
                    if (pid == 0) {//child runs process
                        array = strtoarray(cmdLine);
                        status = execvp("ls", array);
                        free(array);
                        exit(status);
                    } else if(pid > 0) {//parent process
                        waitpid(pid, &status, 0);
                        if (WEXITSTATUS(status) == -1){//if it did not run successfully print error msg
                            perror("ls");
                        }
                    }
                } else {
                    perror("fork");//if fork failed
                }

            } else if (cmdLine[strlen(cmdLine)-1] == '&') {//asynchronous command
                if ((asynchpid = fork()) < 0) {//if fork error
                    perror("fork");
                } else {//if fork was successfull
                    if (asynchpid == 0) {//child runs process
                        cmdLine[strlen(cmdLine)-1] = '\0';//remove & to get the valid array
                        array = strtoarray(cmdLine);//get array for execvp
                        status = execvp(cmdLnArg, array);//execute command
                        if (status == -1) {//if failed to execute command
                            perror(cmdLnArg);
                        }
                        printf(">");
                    
                        free(array);
                        exit(status);
                    } else {//parent process
                        sigact.sa_handler = SIG_DFL;
                        sigaction(SIGQUIT, &sigact, NULL);
                    }
                }

            } else {//none of the previous options
                printf("myShell: %s: command not found\n", cmdLine);
            }
            
        }
    }
    return 0;
}