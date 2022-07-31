#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "HelloShell.h"

#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"


static char **history = NULL; 
static int history_len = 0;

#define INPUT 0
#define OUTPUT 1
#define APPEND 2
#define OPERATION_IN 3
#define OPERATION_OUT 4
#define ASYNCRONOUS 5
#define PWD 6

struct command *parse(char *input);
int is_blank(char *input);
static char *input;
static int* operations;
static int async_number = 0;

int historico(char* input)
{
	const char *chave = "historico";

	if (strlen(input) != strlen(chave))
		return 0;

	for (int i = 0; i < strlen(chave); i++)
	{
		if (input[i] != chave[i])
			return 0;
	}

	return 1;
}

int is_blank(char *input)
{
	int n = (int)strlen(input);
	int i;
	int contador = 0;

	for (i = 0; i < n; i++)
	{
		if (input[i] == ' ')
			contador++;
	}
	
	if(contador == n)
		return 1;
	else
		return 0;
}

void cleanup_and_exit(int status)
{
	// clear_history();
	// free(history);
	// exit(status);
}

struct commands *parse_commands_with_pipes(char *input)
{
	int commandCount = 0;
	int i = 0;
	char *token;
	char *saveptr;
	char *c = input;
	struct commands *cmds;
	operations = (int*) calloc(32,  sizeof(int));
	int operation_number = 0;
	


	while (*c != '\0')
	{
		if (*c == '|')
			commandCount++;
		c++;
	}

	commandCount++;

	cmds = (struct commands *)calloc(sizeof(struct commands) + commandCount * sizeof(struct commands *), 1);

	if (cmds == NULL)
	{
		fprintf(stderr, "error: memory alloc error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok_r(input, "|", &saveptr);
	while (token != NULL && i < commandCount)
	{
		cmds->cmds[i++] = parse(token);
		token = strtok_r(NULL, "|", &saveptr);
	}

	cmds->cmd_count = commandCount;
	return cmds;
}

struct command *parse(char *input)
{

	int tokenCount = 0;
	char *token;

	struct command *cmd = (struct command *)calloc(sizeof(struct command) + ARG_MAX_COUNT * sizeof(char *), 1);

	if (cmd == NULL)
	{
		fprintf(stderr, "error: memory alloc error\n");
		exit(1);
	}

	token = strtok(input, " ");

	while (token != NULL && tokenCount < ARG_MAX_COUNT)
	{
		cmd->argv[tokenCount++] = token;
		token = strtok(NULL, " ");
	}
	cmd->name = cmd->argv[0];
	cmd->argc = tokenCount;
	return cmd;
}

char *read_input(void)
{

	int buffer_size = 2048;
	char *input = (char *)malloc(buffer_size * sizeof(char));
	int i = 0;
	char c;

	if (input == NULL)
	{
		fprintf(stderr, "error: malloc failed\n");
		// cleanup_and_exit(EXIT_FAILURE);
	}

	while ((c = getchar()) != '\n')
	{

		if (c == EOF)
		{
			free(input);
			return NULL;
		}

		if (i >= buffer_size)
		{
			buffer_size = 2 * buffer_size;
			input = (char *)realloc(input, buffer_size);
		}

		input[i++] = c;
	}

	input[i] = '\0';
	return input;
}

int add_to_history(char *input)
{

	char* historic = strdup(input);

	if (history_len == 0)
	{
		history = (char **)calloc(sizeof(char *) * HISTORY_MAXITEMS, 1);

		if (history == NULL)
		{
			fprintf(stderr, "error: memory alloc error\n");
			return 0;
		}

		history[history_len] = historic;
		history_len++;
		return 1;
	}
	else{
		

		if (historic == NULL)
			return 0;

		history[history_len] = historic;
		history_len++;
		return 1;
	}

	return 1;
}
void showHelp(){
	printf(BLUE   "----------Help--------"   RESET "\n");
	printf(BLUE   "Comandos internso: cd, pwd, echo, exit "   RESET "\n");
	printf(BLUE   "A execução por seções de pipe é permitida utilizando ' | ' entre os comandos"   RESET "\n");
	printf(BLUE   "Ex. ls -a | wc -c "   RESET "\n");
	printf(BLUE   "Comandos assincronos podem ser executados com arg[n] =  '&' "   RESET "\n");
	printf(BLUE   "Ex. ls -a &"   RESET "\n"); 
	printf(BLUE   "Redirecionar a saída (OUTPUT) para o arquivo: ls > fileOutput "   RESET "\n");
	printf(BLUE   "Redirecionar o ano (APPEND) para o arquivo: ls >> fileOutput "   RESET "\n");
	printf(BLUE   "Redirecionar a entrada (INPUT): para o arquivO:  wc -c < fileInput "   RESET "\n");
}

int check_built_in(struct command *cmd)
{
	if(strcmp(cmd->name, "exit") == 0)
		return 0;
	else if (strcmp(cmd->name, "cd") == 0 ){
		chdir(cmd->argv[1]);
		return 1;
	}
	else if(strcmp(cmd->name, "historico") == 0 ){

		int historico_numero = 0;

		if(cmd->argv[1] != NULL && !is_blank(cmd->argv[1])){
			historico_numero = atoi(cmd->argv[1]);
			
			if(historico_numero > 10 && historico_numero >=0)
				historico_numero = history_len;
		}
		else
			historico_numero = history_len;


		for (int i = 0; i < historico_numero; i++)
		{
			fprintf(stdout,"%s",history[i]);
			fprintf(stdout, "\n");
		}

		return 1;		
	}
	else if(strcmp(cmd->name, "help") == 0 ){
		showHelp();
	}
	else if(strcmp(cmd->name, "ver") == 0 ){
		fprintf(stdout, GREEN "Versão: 1.0" RESET"\n");
		fprintf(stdout, GREEN"Data de atualização: 31/08/2022" RESET"\n");
		fprintf(stdout, GREEN"Autor: Guilherme Oliveira Loiola" RESET"\n");
	}

	return -1;
}

void close_pipes(int (*pipes)[2], int pipe_count)
{
	int i;

	for (i = 0; i < pipe_count; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

}

int exec_command(struct command *cmd){
	
	fprintf(stdout,"exc. %s", cmd->name);
	int pid = fork();
	if (pid > 0)
		wait(NULL);
	else{
		execvp(cmd->name,cmd->argv);
		exit(EXIT_FAILURE);
	}

	return pid;
}


//TODO: VOID FUCNTIONS
void exec_async( char *argv[], int argc ){

	int pid = fork();
	int size = argc;
	int number = async_number;
	async_number++;

	if(pid==0){
			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);//in case exec is not successfull, exit
	}else{
		fprintf(stdout, "Processo em background [%d] executado\n", number);

		fprintf(stdout ,"Comando :");

		for (int i = 0; i < size-1; i++)
		{
			fprintf(stdout ,"%s ", argv[i]);
		}

		fprintf(stdout ,"&\n");
		
	}

	wait(NULL);

}

int verify_parsing(char * argv){

	if(!strcmp(argv, ">>"))
	{
		return APPEND;
	}else if(!strcmp(argv, ">"))
	{
		return OUTPUT;
	}else if(!strcmp(argv, "<"))
	{
		return INPUT;
	}else if(!strcmp(argv, "pwd"))
	{
		return PWD;
	}
	else if(!strcmp(argv, "&"))
	{
		return ASYNCRONOUS;
	}

	return -1;
}


void exec_redirect(char* path, char *argv[], int mode){

	int fd;
	// char *args[] = calloc(cmds->cmd_count-);
	//TODO: SINCRONIZAR AS ATIVIDADES PARA POSSIBILITAR EXECUÇÃO EM PIPE
	//TODO: especificar o comando dentro de commmands(strcut)
	
	int pid = fork();
	mode_t access = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	if(pid==0){
		//input
		fprintf(stdout, "arg name:%s e path: %s\n", argv[0], path);
		
		if(mode == APPEND){
			fd=open(path,O_RDWR  | O_APPEND| O_CREAT, access);
		}
		else if(mode == INPUT){
			fd=open(path,O_RDONLY, access);
		}
		else if(mode == OUTPUT){
			fd=open(path, O_RDWR | O_CREAT, access);
		}
		
		if(fd<0){
			perror("cannot open file\n");
			return;
		}
		

		if(mode == APPEND || mode == OUTPUT){
			dup2(fd,1);
		}
		else if(mode == INPUT){
			dup2(fd,0);
		}

		close(fd);
		
		// execvp(cmds->cmds[0]->name, token);
		execvp(argv[0], argv);
		
		perror("invalid exec\n");
			return;
	}

	wait(NULL);


}


//working
void exec_piped(struct commands* cmds){


	int numPipes = cmds->cmd_count-1;		
		int i;
		int exec;

		cmds->cmds[0]->fds[STDIN_FILENO] = STDIN_FILENO;

		int fd[10][2];

		for (i = 0; i < cmds->cmd_count; i++)
		{
			if(i!= numPipes)
				if( pipe(fd[i])	< 0)	
					fprintf(stderr, "pipe error\n");

			if(fork()==0){//child1
			if(i!=cmds->cmd_count-1){
				dup2(fd[i][1],1);
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i!=0){
				dup2(fd[i-1][0],0);
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}
			execvp(cmds->cmds[i]->name,cmds->cmds[i]->argv);
			perror("invalid input ");
			exit(1);//in case exec is not successfull, exit
		}
		//parent
		if(i!=0){//second process
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}
		wait(NULL);
		}

		return;

	
}

int exec_commands(struct commands* cmds){

	int pid;

	fprintf(stdout,"\ninput %s :>\ncontador %d",input, cmds->cmd_count);

	if(cmds->cmd_count == 1){
		//SET I/O
		//TODO:
		cmds->cmds[0]->fds[STDIN_FILENO] = STDIN_FILENO;
		cmds->cmds[0]->fds[STDOUT_FILENO] = STDOUT_FILENO;

		pid = exec_command(cmds->cmds[0]);
		wait(NULL);
	}

	return pid;
}

void shellPrompt(){
	// We print the prompt in the form "<user>@<host> <cwd> >"
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	//printf("%s@%s %s > ", getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
}

void welcomeScreen(){
        printf("\n\t============================================\n");
        printf("\t               Simple C Shell\n");
        printf("\t--------------------------------------------\n");
        printf("\t             Aluno: Guilherme Oliveira L\n");
        printf("\t============================================\n");
        printf("\n\n");
}

int main(void)
{
	int exec_ret;
	char c;
	char* input_auxiliar;

	welcomeScreen();

	while (1)
	{

		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		fprintf(stdout,"%s",cwd);
		else 	perror("User path failed\n");

		fputs("$ ", stdout);

		input = read_input();
		input_auxiliar = input;

		if (input == NULL)
		{
			fprintf(stderr, "error: malloc failed\n");
			// cleanup_and_exit(EXIT_FAILURE);
		}
		int i = 0;


		if(strchr(input, '|'))
			fprintf(stdout, "%s ", input);

		if (strlen(input) > 0 && !is_blank(input) && input[0])
		{


			add_to_history(input);

			struct commands *commands = parse_commands_with_pipes(input);
			int operation = 0;
			int option = 0;
			
			
			//TODO: exit, cd, help
			for (int count = i = 0; i < commands->cmd_count; i++)
			{
				count = check_built_in(commands->cmds[i]);
				if (count != -1)
				{
					if(count == 0)
						exit(EXIT_SUCCESS);
				}
				else{
					char *argv[ARG_MAX_COUNT];
					operation = 0;
					option = 0;
					for (int j = 0; j < commands->cmds[i]->argc; j++)
					{
						option = verify_parsing(commands->cmds[i]->argv[j]);
						if(option == APPEND||option == OUTPUT||option == INPUT){
							char* path = commands->cmds[i]->argv[j+1];
							operation = j;
							exec_redirect( path, argv, option);
						}
						else if(option == ASYNCRONOUS){
							char* path = commands->cmds[i]->argv[j+1];
							operation = j;
							exec_async(argv, commands->cmds[i]->argc);
						}
						else{
							if(!operation)
								{argv[j] = commands->cmds[i]->argv[j];
								fprintf(stdout, "arg[%d]: %s\n", j, argv[j]);}
							
						}
					}
					argv[operation] = NULL;
				}
					
				if(operation == 0)
					exec_command(commands->cmds[i]);
			}
				
		}
		

		free(input);

		/* get ready to exit */
		if (exec_ret == -1)
			break;
	}

	
}

