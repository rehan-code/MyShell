#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    int exit = 0;
    char cmdLine[1000];
    char *cmdLnArg;
    char *cmdLnArg2;

    while (exit == 0) {
        printf(">");
        fgets(cmdLine, 1000, stdin);

        if (strlen(cmdLine) == 999 && cmdLine[998] != '\n') {
            while (fgets(cmdLine, 1000, stdin) != NULL) {   
            }
            printf("myShell: too many characters\n");
        } else {
            cmdLine[strlen(cmdLine)-1] = '\0';
            cmdLnArg = strtok(cmdLine, " ");

            if (strcmp(cmdLnArg, "exit")) {
                exit = 1;
                printf("myShell terminating...\n\n");
            } else if (strcmp(cmdLnArg, "ls")) {
                cmdLnArg2 = strtok(NULL, " ");
                if (cmdLnArg2 == NULL) {
                    /* code */
                }
                
            }
            

        }
    return 0;
}