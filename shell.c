#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <time.h>

void setUpParentProcess(char *tokens[5]);
void getInputFromUser(char *tokens[5]);
void splitInput(char *input, char *tokens[5]);
void handlingDollarSign(char *tokens[5]);
void executeCommand(char *tokens[5], int fd);
void changeDirectory(char *path);
void printWorkingDirectory();
void exportVariable(char *string, int fd);
void echo(char *tokens[5]);
void executeExternalCommand(char *tokens[5]);
void splitOnEqual(char *input, char *tokens[2]);
void sleep_ms(int milliseconds);


int main(){
    char *tokens[5];
    system("clear");
    while(1) {
        getInputFromUser(tokens);
        if (strcmp(tokens[0], "exit") == 0) {
            exit(0);
        }
        setUpParentProcess(tokens);
    }
}

void setUpParentProcess(char *tokens[5]){
    pid_t pid;
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }
    pid = fork();
    if(pid == -1){
        printf("Error in forking the process");
        exit(1);
    }
    else if (pid ==0){
        //child process
        close(fd[0]);
        executeCommand(tokens, fd[1]);
    }
    else{
        //parent process
        int status;
        close(fd[1]);
        waitpid(pid,&status,0);
        char buffer[1024];
        read(fd[0], buffer, sizeof(buffer));
        char *exports[2];
        splitOnEqual(buffer, exports);
        if(setenv(exports[0],exports[1], 1) != 0){
            perror("Error in setting environment variable");
        }
        close(fd[0]); // Close read end
    }
}

void getInputFromUser(char *tokens[5]){
    char input [PATH_MAX];
    char currentDirectory [PATH_MAX];
    if (getcwd(currentDirectory, sizeof(currentDirectory)) != NULL){
        sleep_ms(20);
        printf("%s:", currentDirectory);
        fgets(input, sizeof(input),stdin);
        splitInput(input, tokens);
        handlingDollarSign(tokens);
    }
    else{
        perror("Error in getting current directory");
    }
}
void handlingDollarSign(char *tokens[5]){
    for(int i = 0; i < 5; i++){
        if(tokens[i] != NULL){
            if(tokens[i][0] == '$'){
                char *env = getenv(tokens[i]+1);
                if(env != NULL){
                    tokens[i] = env;
                }
            }
        }
    }
}

void splitInput(char *input, char *tokens[5]){
    char *token;
    int count = 0;
    token = strtok(input, " \n");
    while(token != NULL && count < 4){
        tokens[count] = token;
        count++;
        token = strtok(NULL, " \n");
    }
    tokens[count] = NULL;
}

void executeCommand(char *tokens[5], int fd){
    char *specialCommands[] = {"cd", "pwd", "export", "echo",NULL};
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
            break;
        case 1:
            printWorkingDirectory();
            break;
        case 2:
            exportVariable(tokens[1], fd);
            break;
        case 3:
            echo(tokens);
            break;
        default:
            executeExternalCommand(tokens);
    }
}

void changeDirectory(char *path) {
    char resolvedPath[PATH_MAX];
    if (strcmp(path, "..") == 0) {
        if (chdir("..") != 0) {
            perror("chdir() error");
            exit(1);
        }
    } else if (strcmp(path, "~") == 0) {
        const char *homeDir = getenv("HOME");
        if (homeDir == NULL) {
            fprintf(stderr, "HOME environment variable not set\n");
            exit(1);
        }
        if (chdir(homeDir) != 0) {
            perror("chdir() error");
            exit(1);
        }
    } else {
        if (realpath(path, resolvedPath) == NULL) {
            if (chdir(path) != 0) {
                perror("chdir() error");
                exit(1);
            }
        } else {
            if (chdir(resolvedPath) != 0) {
                perror("chdir() error");
                exit(1);
            }
        }
    }
    exit(0);
}
void printWorkingDirectory(){
    char currentDirectory[1024];
    if(getcwd(currentDirectory, sizeof(currentDirectory)) != NULL){
        printf("%s\n", currentDirectory);
        exit(0);
    }
    else{
        perror("Error in getting current directory");
        exit(1);
    }
}
void exportVariable(char *string, int fd){
    char *tokens[2];
    splitOnEqual(string, tokens);
    if(setenv(tokens[0],tokens[1], 1) != 0){
        perror("Error in setting environment variable");
        exit(1);
    }
    else{
        write(fd, string, strlen(string));
        exit(0);
    }
}
void echo(char *tokens[5]){
    for(int i = 1; tokens[i] != NULL; i++){
        printf("%s ", tokens[i]);
    }
    printf("\n");
    exit(0);
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
void splitOnEqual(char *input, char *tokens[2]){
    char *token;
    int count = 0;
    token = strtok(input, "=");
    while(token != NULL && count < 2){
        tokens[count] = token;
        count++;
        token = strtok(NULL, "=");
    }
    tokens[count] = NULL;
}

