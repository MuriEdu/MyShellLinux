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

//TRATAMENTO DE COMANDOS RECEBIDOS NO PROMPT
void inputHandling(char * input){
	char *prog;
	char *argv[MAX_ARGS];
    int argCounter = 0;
    char *arg;
    int lastArg = 0;

    arg = strtok(input, " ");
    while (arg != NULL && argCounter < MAX_ARGS - 1){
        if(strcmp(arg, "&") == 0)
            lastArg = 1;
        else {
            argv[argCounter] = arg;
            argCounter++;
        }
        arg = strtok(NULL, " ");
    }

    prog = argv[0];
    argv[argCounter] = NULL;


	if(strcmp(prog, "exit") == 0) exit(EXIT_SUCCESS);

	pid_t pid = fork();
	switch(pid){
		case -1:
			perror("fork error in inputHandling");
			exit(EXIT_FAILURE);
		case 0: //child code
			if(execvp(prog, argv) == -1) {
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
