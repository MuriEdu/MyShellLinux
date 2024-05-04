#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


void shellPrompt();
void initPromptPrint();
void initShell();
void inputHandling(char * input);

#define BUFFER_LENGTH 50
#define MAX_ARGS 10

//FUNCAO PARA INICIAR O SHELL
void initShell(){

	printf("WELLCOME TO MURI'S SHELL\n");
    initPromptPrint();
	shellPrompt();
}

//FUNCAO PARA PRINTAR INDICACAO DE DIGITACAO
void initPromptPrint() {
    printf("#> ");
}

//PROMPT DO SHELL
void shellPrompt(){

	char buffer[BUFFER_LENGTH];
	int exitSig = 0;

	do {
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';
		inputHandling(buffer);
	}while(exitSig == 0);

}
//ROTINA DE TRATAMENDO DO SINAL SIGCHLD
void sigchildHandler(int signalNum){
    wait(NULL);
}

//TRATAMENTO DE COMANDOS RECEBIDOS NO PROMPT
void inputHandling(char * input){
	char *prog;
    char *progAux;
	char *argv[MAX_ARGS];
    char *argvAux[MAX_ARGS];
    int argCounter = 0;
    int argCounterAux = 0;
    char *arg;
    char *destFileStr;
    int lastArg = 0;
    int redirectOutput = 0;
    int isPipe = 0;
    int pipefd[2];

    arg = strtok(input, " ");
    while (arg != NULL && argCounter < MAX_ARGS - 1){
        //VERIFICACAO DE CARACTERES ESPECIAIS


        if(strcmp(arg, "&") == 0)
            lastArg = 1;
        else if(strcmp(arg, ">") == 0){
            destFileStr = strtok(NULL, " ");
            redirectOutput = 1;
        } else if(strcmp(arg, "|") == 0){
            arg = strtok(NULL, " ");
            while (arg != NULL && argCounterAux < MAX_ARGS - 1) {
                argvAux[argCounterAux] = arg;
                argCounterAux++;
                arg = strtok(NULL, " ");
            }
            isPipe = 1;
        }
        else {
            argv[argCounter] = arg;
            argCounter++;
        }
        arg = strtok(NULL, " ");
    }

    prog = argv[0];
    argv[argCounter] = NULL;

    progAux = argvAux[0];
    argvAux[argCounterAux] = NULL;


	if(strcmp(prog, "exit") == 0) exit(EXIT_SUCCESS);

    signal(SIGCHLD, sigchildHandler);

    //TRATAMENTO DO COMANDO COM PIPE
    if (isPipe) {
        pipe(pipefd);
        pid_t pid1 = fork();
        switch (pid1) {
            case -1:
                perror("fork error in inputHandling");
                exit(EXIT_FAILURE);
            case 0:
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                if (execvp(prog, argv) == -1) {
                    perror("error trying to execute your program");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                break; // Não faz nada no processo pai
        }
        pid_t pid2 = fork();
        switch (pid2) {
            case -1:
                perror("fork error in inputHandling");
                exit(EXIT_FAILURE);
            case 0:
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);
                if (execvp(progAux, argvAux) == -1) {
                    perror("error trying to execute your program");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
        }
    }

    //TRATAMENTO DE UM COMANDO SEM PIPE
    pid_t pid;
    if(isPipe) pid = -2;
    else pid = fork();
	switch(pid){
        case -2:
            break;
		case -1:
			perror("fork error in inputHandling");
			exit(EXIT_FAILURE);
		case 0: //child code
            if(redirectOutput) {
                FILE *destFile = fopen(destFileStr, "w");
                if (destFile != NULL) {
                    int fd = fileno(destFile);
                    dup2(fd, STDOUT_FILENO);
                    if(execvp(prog, argv) == -1) {
                        perror("error trying to execute your program");
                        exit(EXIT_FAILURE);
                    }
                    fd = fileno(stdout);
                    dup2(fd, STDOUT_FILENO);
                    fclose(destFile);
                }
            }
			else if(execvp(prog, argv) == -1) {
                perror("error trying to execute your program");
                exit(EXIT_FAILURE);
            }
            break;
		default:
            if(lastArg == 0)
                wait(NULL);
	}

    initPromptPrint();
}

int main(){
    initShell();
    return 0;
}
