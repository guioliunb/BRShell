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


static char **history = NULL; /* array of strings for storing history */
static int history_len = 0;

#define INPUT 0
#define OUTPUT 1
#define APPEND 2

struct command *parse(char *input);
int is_blank(char *input);
static char *input;

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

void check_built_in(struct command *cmd)
{
	if(strcmp(cmd->name, "exit") == 0)
		exit(0);
	else if (strcmp(cmd->name, "cd") == 0 ){
		chdir(cmd->argv[1]);
	}
}

void close_pipes(int (*pipes)[2], int pipe_count)
{
	int i;

	for (i = 0; i < pipe_count; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

}

int exec_command(struct commands *cmds, struct command *cmd, int (*pipes)[2]){
	

	fprintf(stdout,"%s", cmd->name);
	int pid = fork();
	if (pid > 0)
		wait(NULL);
	else{
		execvp(cmd->name,cmd->argv);
		exit(EXIT_FAILURE);
	}

	return pid;
}

void exec_async(struct commands* cmds){
	
}

void exec_append(struct commands* cmds){

	int fd;
	//TODO: SINCRONIZAR AS ATIVIDADES PARA POSSIBILITAR EXECUÇÃO EM PIPE
	//TODO: especificar o comando dentro de commmands(strcut)
	fprintf(stdout, "path %s\n", cmds->cmds[0]->argv[2]);
	// fprintf(stdout, "path %s\n", cmds->cmds[0]->argv[2]);

	if(fork()==0){
		//input
		fd = open(cmds->cmds[0]->argv[2],O_WRONLY | O_APPEND);

		fprintf(stdout, "fd %d\n", fd);
		
		dup2(fd,1);

		fprintf(stdout, "fd lalal %d\n", fd);
		
		execvp(cmds->cmds[0]->argv[0], cmds->cmds[0]->argv+1);
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
		cmds->cmds[0]->fds[STDIN_FILENO] = STDIN_FILENO;
		cmds->cmds[0]->fds[STDOUT_FILENO] = STDOUT_FILENO;

		pid = exec_command(cmds, cmds->cmds[0], NULL);
		wait(NULL);
	}
	else{
		

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
			int historico_numero = 0;

			//exit, cd, help
			check_built_in(commands->cmds[0]);

			if(historico(commands->cmds[0]->name)){

				if(commands->cmds[0]->argv[1] != NULL && !is_blank(commands->cmds[0]->argv[1])){
					historico_numero = atoi(commands->cmds[0]->argv[1]);
					
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
				}
				else{
					//exec_piped(commands);
					exec_append(commands);

				}	

		

		free(input);

		/* get ready to exit */
		if (exec_ret == -1)
			break;
	}

	
}

}