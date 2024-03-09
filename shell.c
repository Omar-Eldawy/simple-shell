#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <time.h>

#define TOKEN_SIZE 20


void setUpParentProcess(char *tokens[TOKEN_SIZE], int *runInBackground);
void getInputFromUser(char *tokens[TOKEN_SIZE], int *runInBackground);
void splitInput(char *input, char *tokens[TOKEN_SIZE ]);
void handlingDollarSign(char *tokens[TOKEN_SIZE ]);
int executeCommand(char *tokens[TOKEN_SIZE ]);
void changeDirectory(char *path);
void printWorkingDirectory();
void exportVariable(char *string[TOKEN_SIZE]);
void echo(char *tokens[TOKEN_SIZE ]);
void executeExternalCommand(char *tokens[TOKEN_SIZE ]);
void splitOnEqual(char *input[TOKEN_SIZE ], char *tokens[2]);
int checkQuotes(char *input[TOKEN_SIZE]);
void sleep_ms(int milliseconds);
int getFirstEqualSign(char *input);
void setEnvironmentVariable(char *variables[2]);
void concatenateString(char *string[TOKEN_SIZE]);
void removeQuotes(char *string);
void shiftStringOneStepBackWards(char *string);
void handlingDollarSign2(char *token);

int main(){
    char *tokens[TOKEN_SIZE];
    system("clear");
    while(1) {
        int runInBackground = 0;
        getInputFromUser(tokens, &runInBackground);
        int flag = executeCommand(tokens);
        if (flag){
            continue;
        }
        setUpParentProcess(tokens, &runInBackground);
    }
}

void setUpParentProcess(char *tokens[5], int *runInBackground){
    pid_t pid;
    pid = fork();
    if(pid == -1){
        printf("Error in forking the process");
        exit(1);
    }
    else if (pid ==0){
        //child process
        executeExternalCommand(tokens);
    }
    else{
        //parent process
        int status;
        if(*runInBackground == 0) {
            waitpid(pid, &status, 0);
        }
    }
}

void getInputFromUser(char *tokens[5], int *runInBackground){
    char input [PATH_MAX];
    char currentDirectory [PATH_MAX];
    if (getcwd(currentDirectory, sizeof(currentDirectory)) != NULL){
        sleep_ms(30);
        printf("%s:", currentDirectory);
        fgets(input, sizeof(input),stdin);
        if(input[strlen(input) - 2] == '&'){
            *runInBackground = 1;
            input[strlen(input) - 3] = '\0';
        }
        for (int j = 0; input[j] != '\0'; j++) {
            if (input[j] == '$') {
                char *beforeDollarSign = strndup(input, j);
                char envVariableName[2] = {input[j + 1], '\0'}; // Take only the first character after $
                int removedQuotation = 0;
                if (input[strlen(input) - 2] == '\"') {
                    removedQuotation = 1;
                }
                char *envVariableValue = getenv(envVariableName);
                if (envVariableValue != NULL) {
                    char *newToken = malloc(strlen(beforeDollarSign) + strlen(envVariableValue) + 1);
                    strcpy(newToken, beforeDollarSign);
                    strcat(newToken, envVariableValue);
                    if (removedQuotation) {
                        strcat(newToken, "\"");
                    }
                    strcpy(input, newToken);
                } else {
                    printf("Error in getting environment variable\n");
                }
                free(beforeDollarSign);
            }
        }
        splitInput(input, tokens);
    }
    else{
        perror("Error in getting current directory");
    }
}
void handlingDollarSign(char *tokens[TOKEN_SIZE]) {
    for (int i = 0; tokens[i] != NULL; i++) {
        char *token = tokens[i];
        for (int j = 0; token[j] != '\0'; j++) {
            if (token[j] == '$') {
                char *beforeDollarSign = strndup(token, j);
                char *envVariableName = strdup(token + j + 1);
                int removedQuotation = 0;
                if (envVariableName[strlen(envVariableName) - 1] == '"') {
//                    envVariableName[strlen(envVariableName) - 1] = '\0';
                    removedQuotation = 1;
                }
                char *envVariableValue = getenv(envVariableName);
                if (envVariableValue != NULL) {
                    char *newToken = malloc(strlen(beforeDollarSign) + strlen(envVariableValue) + 1);
                    strcpy(newToken, beforeDollarSign);
                    strcat(newToken, envVariableValue);
                    if (removedQuotation) {
                        strcat(newToken, "\"");
                    }
                    tokens[i] = newToken;
                } else {
                    printf("Error in getting environment variable\n");
                }
                free(beforeDollarSign);
                free(envVariableName);
            }
        }
    }
}

void splitInput(char *input, char *tokens[TOKEN_SIZE]){
    char *token;
    int count = 0;
    token = strtok(input, " \n");
    while(token != NULL && count < TOKEN_SIZE - 1){
        tokens[count] = token;
        count++;
        token = strtok(NULL, " \n");
    }
    tokens[count] = NULL;
}

int executeCommand(char *tokens[TOKEN_SIZE ]){
    char *specialCommands[] = {"cd", "pwd", "export", "echo","exit",NULL};
    int type = sizeof specialCommands / sizeof specialCommands[0] - 1;
    for(int i = 0; specialCommands[i] != NULL; i++){
        if(strcmp(tokens[0], specialCommands[i]) == 0){
            type = i;
            break;
        }
    }
    switch(type){
        case 0:
            changeDirectory(tokens[1]);
            return 1;
        case 1:
            printWorkingDirectory();
            return 1;
        case 2:
            exportVariable(tokens);
            return 1;
        case 3:
            echo(tokens);
            return 1;
        case 4:
            exit(0);
        default:
            return 0;
    }
}

void changeDirectory(char *path) {
    char resolvedPath[PATH_MAX];
    if (strcmp(path, "..") == 0) {
        if (chdir("..") != 0) {
            perror("chdir() error");
            return;
        }
    } else if (strcmp(path, "~") == 0) {
        const char *homeDir = getenv("HOME");
        if (homeDir == NULL) {
            fprintf(stderr, "HOME environment variable not set\n");
            return;
        }
        if (chdir(homeDir) != 0) {
            perror("chdir() error");
            return;
        }
    } else {
        if (realpath(path, resolvedPath) == NULL) {
            if (chdir(path) != 0) {
                perror("chdir() error");
                return;
            }
        } else {
            if (chdir(resolvedPath) != 0) {
                perror("chdir() error");
                return;
            }
        }
    }
}
void printWorkingDirectory(){
    char currentDirectory[1024];
    if(getcwd(currentDirectory, sizeof(currentDirectory)) != NULL){
        printf("%s\n", currentDirectory);
        return;
    }
    else{
        perror("Error in getting current directory");
        return;
    }
}
void exportVariable(char *string [TOKEN_SIZE]){
    char *variables[2];
    int afterEqual = getFirstEqualSign(string[1]);
    //string without spaces
    if(afterEqual == 0){
        splitOnEqual(string, variables);
        setEnvironmentVariable(variables);
        return;
    }
    //string inside quotes
    else if(afterEqual == 1) {
        int flag = checkQuotes(string);
        if (flag == 0) {
            printf("Error in quotes\n");
            return;
        } else {
        concatenateString(string);
        removeQuotes(string[1]);
        splitOnEqual(string, variables);
        shiftStringOneStepBackWards(variables[1]);
        int i = strlen(variables[1])-1;
        while(variables[1][i] != ' '){
            i++;
        }
        variables[1][i] = '\0';
        setEnvironmentVariable(variables);
        return;
        }
    }
    else{
        return;
    }
}
void echo(char *tokens[TOKEN_SIZE]){
    int flag = checkQuotes(tokens);
    if(flag == 0){
        printf("Error in quotes\n");
        return;
    }
    else{
        for(int i = 1; tokens[i] != NULL; i++){
            removeQuotes(tokens[i]);
        }
    }
    for(int i = 1; tokens[i] != NULL; i++){
        printf("%s ", tokens[i]);
    }
    printf("\n");
}
void executeExternalCommand(char *tokens[5]){
    execvp(tokens[0], tokens);
    perror("Error in executing command");
    exit(1);
}
void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
void splitOnEqual(char *input[TOKEN_SIZE], char *tokens[2]){
    char *token;
    int count = 0;
    token = strtok(input[1], "=");
    while(token != NULL && count < 2){
        tokens[count] = token;
        count++;
        token = strtok(NULL, "=");
    }
    tokens[count] = NULL;
}
int checkQuotes(char *input[TOKEN_SIZE]){
    int count = 0;
    for (int i=1; input[i] != NULL; i++){
       for (int j=0; input[i][j] != '\0'; j++){
           if (input[i][j] == '"'){
               count++;
           }
       }
    }
    if(count % 2 == 0){
        return 1;
    }
    else{
        return 0;
    }
}
int getFirstEqualSign(char *input) {
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '=') {
            if (input[i + 1] == '"') {
                return 1;
            }
            else {
                return 0;
            }
        }
    }
    return 2;
}
void setEnvironmentVariable(char *variables[2]){
    if(setenv(variables[0],variables[1], 1) != 0){
        perror("Error in setting environment variable");
        return;
    }
    else{
        return;
    }
}
void concatenateString(char *string[TOKEN_SIZE]){
    char whitespace[] = " ";
    unsigned int totalLength = strlen(string[1]);
    for(int i = 2; string[i] != NULL; i++){
        totalLength += strlen(string[i]) + 1; // +1 for the whitespace
    }
    char *newString = malloc(totalLength + 1); // +1 for the null terminator
    if(newString == NULL){
        perror("Error allocating memory");
        return;
    }
    strcpy(newString, string[1]);
    for(int i = 2; string[i] != NULL; i++){
        strcat(newString, whitespace);
        strcat(newString, string[i]);
    }
    string[1] = newString;
}
void removeQuotes(char *string){
    int i = 0;
    while(string[i] != '\0'){
        if(string[i] == '"'){
            string[i] = ' ';
        }
        i++;
    }
}
void shiftStringOneStepBackWards(char *string){
    int i = 0;
    while(string[i] != '\0'){
        string[i] = string[i+1];
        i++;
    }
}
void handlingDollarSign2(char *token){
    for (int j = 0; token[j] != '\0'; j++) {
        if (token[j] == '$') {
            char *beforeDollarSign = strndup(token, j);
            char *envVariableName = strdup(token + j + 1);
            int removedQuotation = 0;
            if (envVariableName[strlen(envVariableName) - 2] == '\"') {
                envVariableName[strlen(envVariableName) - 2] = '\0';
                removedQuotation = 1;
            }
            char *envVariableValue = getenv(envVariableName);
            if (envVariableValue != NULL) {
                char *newToken = malloc(strlen(beforeDollarSign) + strlen(envVariableValue) + 1);
                strcpy(newToken, beforeDollarSign);
                strcat(newToken, envVariableValue);
                if (removedQuotation) {
                    strcat(newToken, "\"");
                }
                token = newToken;
            } else {
                printf("Error in getting environment variable\n");
            }
            free(beforeDollarSign);
            free(envVariableName);
        }
    }
}