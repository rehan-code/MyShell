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

void redOutProcess(char **cmdline, int i) {
    int status;
    
    pid_t pid = fork();
    if(pid >= 0) {//if valid
        if (pid == 0) {//child runs process
            FILE *fp = freopen(cmdline[i+1], "w", stdout);// set the new output stream
            if (fp == NULL) {
                perror("Error");
                return;
            }
            
            cmdline[i] = NULL;
            status = execvp(cmdline[0], cmdline);
            fclose(fp);
            exit(status);
        } else if(pid > 0) {//parent process
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status) == -1){//if it did not run successfully print error msg
                perror(cmdline[0]);
            }
        }
    } else {
        perror("fork");//if fork failed
    }
}

void redInpProcess(char **cmdline, int i) {
    int status;
    
    pid_t pid = fork();
    if(pid >= 0) {//if valid
        if (pid == 0) {//child runs process
            FILE *fp = freopen(cmdline[i+1], "r", stdin);// set the new input stream
            
            if (fp == NULL) {
                perror("Error");
                return;
            }
            cmdline[i] = NULL;
            status = execvp(cmdline[0], cmdline);
            fclose(fp);
            exit(status);
        } else if(pid > 0) {//parent process
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status) == -1){//if it did not run successfully print error msg
                perror(cmdline[0]);
            }
        }
    } else {
        perror("fork");//if fork failed
    }
}

void pipeProcess(char **cmdline, int i) {
    int status, status2;
    int pipefd[2];

    if (pipe(pipefd)==-1) {
        perror("pipe");
        return;
    }
    
    pid_t pid = fork();
    if(pid >= 0) {//if valid
        if (pid == 0) {//child runs write process
            dup2(pipefd[1],STDOUT_FILENO);
            close(pipefd[0]);
            //execvp and close
            cmdline[i] = NULL;
            status = execvp(cmdline[0], cmdline);//execute output command
            close(pipefd[1]);
            exit(status);
        } else if(pid > 0) {//parent process

            pid_t pid2 = fork();//second fork
            if(pid2 >= 0) {//if valid
                if (pid2 == 0) {//2nd child runs read process
                    dup2(pipefd[0],STDIN_FILENO);
                    close(pipefd[1]);
                    //make new array for the execvp
                    int j;
                    char **newArray = malloc(sizeof(char*)*20);
                    for (j = i; cmdline[j] != NULL; j++) {
                        strcpy(newArray[j-i],cmdline[j]);
                    }
                    newArray[j] = NULL;
                    //execvp and close
                    status = execvp(newArray[0], newArray);
                    close(pipefd[0]);
                    free(newArray);
                    exit(status);
                } else if(pid2 > 0) {//parent process
                    close(pipefd[1]);
                    close(pipefd[0]);
                    waitpid(pid, &status, 0);
                    if (WEXITSTATUS(status) == -1){//if it did not run successfully print error msg
                        perror(cmdline[0]);
                    }
                    waitpid(pid2, &status2, 0);
                    if (WEXITSTATUS(status2) == -1){//if it did not run successfully print error msg
                        perror(cmdline[i+1]);
                    }
                }
            } else {
                close(pipefd[1]);
                close(pipefd[0]);
                perror("fork");//if fork failed
                waitpid(pid, &status, 0);
                return;
            }
        }
    } else {
        perror("fork");//if fork failed
    }
}

void runProfileCmds(FILE *profilefileptr,char *path, char *histfile, char *home) {
    char cmdline[1000], temp[1000];
    char *cmdLnArg, *cmdLnArg2, *substring;
    int i;

    while (fgets(cmdline, 1000, profilefileptr)!=NULL) {
        cmdline[strlen(cmdline)] = '\0';
        if (cmdline[strlen(cmdline)-1] == '\n') {
            cmdline[strlen(cmdline)-1] = '\0';
        }
        
        cmdLnArg = strtok(cmdline, " \t");
        cmdLnArg2 = strtok(NULL, " \t");

        if (strcmp(cmdLnArg, "export")==0) {
            //replace the environment variables with the values
            strcpy(temp,cmdLnArg2);
            if ((substring = strstr(cmdLnArg2, "$PATH")) != NULL) {
                strcpy(temp, cmdLnArg2);
                for (i = (strlen(cmdLnArg2)-strlen(substring)); i < 1000; i++) {
                    temp[i]='\0';
                }
                strcat(temp, path);
                strcat(temp, substring+5);
                strcpy(cmdLnArg2, temp);
            } else if ((substring = strstr(cmdLnArg2, "$HOME")) != NULL) {
                strcpy(temp, cmdLnArg2);
                for (i = (strlen(cmdLnArg2)-strlen(substring)); i < 1000; i++) {
                    temp[i]='\0';
                }
                strcat(temp, home);
                strcat(temp, substring+5);
                strcpy(cmdLnArg2, temp);
            } else if ((substring = strstr(cmdLnArg2, "$HISTFILE")) != NULL) {
                strcpy(temp, cmdLnArg2);
                for (i = (strlen(cmdLnArg2)-strlen(substring)); i < 1000; i++) {
                    temp[i]='\0';
                }
                strcat(temp, histfile);
                strcat(temp, substring+9);
                strcpy(cmdLnArg2, temp);
            }

            //print the env variable values
            if (strncmp(cmdLnArg2, "PATH=", 5)==0) {
                strcpy(path, cmdLnArg2+5);
            } else if (strncmp(cmdLnArg2, "HOME=", 5)==0) {
                strcpy(home, cmdLnArg2+5);
            } else if (strncmp(cmdLnArg2, "HISTFILE=", 9)==0) {
                strcpy(histfile, cmdLnArg2+9);
            }
        }
    }
}


int main(int argc, char **argv) {
    int exitcmd = 0;
    char cmdLine[1000], cmdLineTokens[1000], cmdLineTokens2[1000], histline[1000], cwd[1000], temp[1000];
    char *substring;
    char *cmdLnArg;
    char *cmdLnArg2;
    pid_t pid;
    pid_t asynchpid;
    char **array;
    int status, i;
    int stploop = 0;
    int histLnNo = 0;
    char c;

    //set and intialize environment variables
    char *path = malloc(1000);
    strcpy(path, "/bin");
    char *home = malloc(1000);
    home = getcwd(home,1000);
    getcwd(cwd, 1000);
    char *histfile = malloc(1000);
    strcpy(histfile, home);
    strcat(histfile, "/.CIS3110_history");
    FILE *histfileptr = fopen(histfile, "a+");
    if (histfileptr == NULL) {
        perror("histfileptr");
    }
    //get the line no. of the histfile (used when adding new entrees)
    fseek(histfileptr, 0, SEEK_END);
    if (ftell(histfileptr)==0) {
        histLnNo = 0;
    } else {
        fseek(histfileptr, 0, SEEK_SET);
        for (c = getc(histfileptr); c!=EOF; c = getc(histfileptr)) {
            if (c == '\n') {
                histLnNo++;
            }
        }
    }
    //initialize and run commands in profile file
    FILE *profilefileptr = fopen(".CIS3110_profile", "a+");
    if (profilefileptr == NULL) {
        perror("profilefileptr");
    } else {
        runProfileCmds(profilefileptr, path, histfile, home);
        fclose(profilefileptr);
    }

    //initialize sigaction
    struct sigaction sigact;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);

    sigact.sa_handler = sigquit;
    if (sigaction(SIGQUIT, &sigact, NULL) < 0) {
        perror("sigaction()");
        exit(1);
    }

    while (exitcmd == 0) {//loop until exit is entered
        stploop = 0;
        printf("%s> ",cwd);
        fgets(cmdLine, 1000, stdin);

        //checking if there are too many character in the command line
        if (strlen(cmdLine) == 999 && cmdLine[998] != '\n') {
            while (fgets(cmdLine, 1000, stdin) != NULL) {
            }
            printf("myShell: too many characters\n");

        } else {//if valid no. of arguments, run rest of the program
            //add new command to the history file
            fseek(histfileptr, 0, SEEK_END);
            fprintf(histfileptr, " %d  %s", ++histLnNo, cmdLine);
            cmdLine[strlen(cmdLine)-1] = '\0';//remove the \n character
            strcpy(cmdLineTokens, cmdLine);

            //get the first and second arg from the argline
            cmdLnArg = strtok(cmdLineTokens, " \t");
            cmdLnArg2 = strtok(NULL, " \t");
            strcpy(cmdLineTokens2, cmdLine);

            


            //To run redirection pipe commands
            array = strtoarray(cmdLineTokens2);
            for (i = 0; array[i]!=NULL && stploop == 0; i++) {
                if (strcmp(array[i], ">")==0) {//To change the output stream
                    redOutProcess(array, i);
                    stploop = 1;
                } else if (strcmp(array[i], "<")==0) {//change the input stream
                    redInpProcess(array, i);
                    stploop = 1;
                } else if (strcmp(array[i], "|")==0) {//pipe
                    pipeProcess(array, i);
                    stploop = 1;
                }
            }
            free(array);


            if (stploop == 0) {//loop to run all other commands

                if (strcmp(cmdLnArg, "exit")==0) {//exit command
                    exitcmd = 1;
                    printf("myShell terminating...\n\n");
                    free(path);
                    free(histfile);
                    free(home);
                    fclose(histfileptr);
                    kill(asynchpid,SIGQUIT);

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
                            printf("> ");
                        
                            free(array);
                            exit(status);
                        } else {//parent process
                            sigact.sa_handler = SIG_DFL;
                            sigaction(SIGQUIT, &sigact, NULL);
                        }
                    }

                //the 3 echo options for path, histfile and home
                }else if (strcmp(cmdLnArg, "echo")==0 && strcmp(cmdLnArg2, "$PATH")==0) {
                    printf("%s\n", path);

                }else if (strcmp(cmdLnArg, "echo")==0 && strcmp(cmdLnArg2, "$HISTFILE")==0) {
                    printf("%s\n", histfile);

                }else if (strcmp(cmdLnArg, "echo")==0 && strcmp(cmdLnArg2, "$HOME")==0) {
                    printf("%s\n", home);

                }else if (strcmp(cmdLnArg, "history")==0) {//history command
                    if (cmdLnArg2!=NULL && strcmp(cmdLnArg2, "-c")==0) {//-c clears history file
                        histfileptr = freopen(histfile, "w+", histfileptr);//reopen the file causes the file to be erased
                        histLnNo = 0;

                    } else if (cmdLnArg2!=NULL && atoi(cmdLnArg2)!=0) {//n prints last n lines
                        
                        fseek(histfileptr, -1, SEEK_END);
                        for (c = getc(histfileptr), i=0; i!=atoi(cmdLnArg2)+1 && i<=histLnNo; c = getc(histfileptr)) {
                            if (c == '\n') {
                                i++;
                            }
                            fseek(histfileptr, -2, SEEK_CUR);
                        }
                        fseek(histfileptr, 1, SEEK_CUR);
                        if (i==histLnNo) {
                            fseek(histfileptr, 0, SEEK_SET);
                        }
                        
                        while (fgets(histline, 1000, histfileptr)!=NULL) {
                            printf("%s", histline);
                        }

                    } else {
                        fseek(histfileptr, 0, SEEK_SET);
                        while (fgets(histline, 1000, histfileptr)!=NULL) {
                            printf("%s", histline);
                        }
                    }

                }else if (strcmp(cmdLnArg, "export")==0) {
                    //replace the environment variables with the values
                    strcpy(temp,cmdLnArg2);
                    if ((substring = strstr(cmdLnArg2, "$PATH")) != NULL) {
                        strcpy(temp, cmdLnArg2);
                        for (i = (strlen(cmdLnArg2)-strlen(substring)); i < 1000; i++) {
                            temp[i]='\0';
                        }
                        strcat(temp, path);
                        strcat(temp, substring+5);
                        strcpy(cmdLnArg2, temp);
                    } else if ((substring = strstr(cmdLnArg2, "$HOME")) != NULL) {
                        strcpy(temp, cmdLnArg2);
                        for (i = (strlen(cmdLnArg2)-strlen(substring)); i < 1000; i++) {
                            temp[i]='\0';
                        }
                        strcat(temp, home);
                        strcat(temp, substring+5);
                        strcpy(cmdLnArg2, temp);
                    } else if ((substring = strstr(cmdLnArg2, "$HISTFILE")) != NULL) {
                        strcpy(temp, cmdLnArg2);
                        for (i = (strlen(cmdLnArg2)-strlen(substring)); i < 1000; i++) {
                            temp[i]='\0';
                        }
                        strcat(temp, histfile);
                        strcat(temp, substring+9);
                        strcpy(cmdLnArg2, temp);
                    }

                    //print the env variable values
                    if (strncmp(cmdLnArg2, "PATH=", 5)==0) {
                        strcpy(path, cmdLnArg2+5);
                    } else if (strncmp(cmdLnArg2, "HOME=", 5)==0) {
                        strcpy(home, cmdLnArg2+5);
                    } else if (strncmp(cmdLnArg2, "HISTFILE=", 9)==0) {
                        strcpy(histfile, cmdLnArg2+9);
                    }
                    
                }else if (strcmp(cmdLnArg, "cd")==0) {//cd function
                    if(chdir(cmdLnArg2)==0){
                        getcwd(cwd,1000);
                    } else {
                        perror("cd");
                    }

                } else {//execute a command
                    pid = fork();
                    if(pid >= 0) {//if valid
                        if (pid == 0) {//child runs process
                            array = strtoarray(cmdLine);
                            status = execvp(array[0], array);
                            free(array);
                            exit(status);
                        } else {//parent process
                            waitpid(pid, &status, 0);
                            if (WEXITSTATUS(status) == -1){//if it did not run successfully print error msg
                                perror(cmdLine);
                            }
                        }
                    } else {
                        perror("fork");//if fork failed
                    }
                }
            }    
        }
    }
    return 0;
}