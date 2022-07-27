#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "HelloShell.h"


static char **history = NULL; /* array of strings for storing history */
static int history_len = 0;


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

int exec_command(struct commands *cmds, struct command *cmd, int (*pipes)[2]){


	pid_t pid = fork();
	if(pid >0){
		wait(NULL);
	}else{
		//child
		execvp(cmd->argv[0],cmd->argv);
		//in case exec is not successfull, exit
		
	}

	return pid;
}

int exec_commands(struct commands* cmds){

	int pid;

	if(cmds->cmd_count == 1){
		//SET I/O
		cmds->cmds[0]->fds[STDIN_FILENO] = STDIN_FILENO;
		cmds->cmds[0]->fds[STDOUT_FILENO] = STDOUT_FILENO;

		pid = exec_command(cmds, cmds->cmds[0], NULL);
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

	welcomeScreen();

	while (1)
	{

		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		fprintf(stdout,"%s",cwd);
		else 	perror("User path failed\n");

		fputs("$ ", stdout);

		input = read_input();

		if (input == NULL)
		{
			fprintf(stderr, "error: malloc failed\n");
			// cleanup_and_exit(EXIT_FAILURE);
		}
		int i = 0;

		// while ( input[i] != '\0')
		// {
		//     cout << input[i];
		//     i++;
		// }

		// fprintf(stdout, "%d\n", strlen(input));
		// fprintf(stdout, "negado %d\n", is_blank(input));
		// fprintf(stdout, "%c\n", input[0]);

		if (strlen(input) > 0 && !is_blank(input) && input[0])
		{
			//char* linecopy = strdup(input);

			// fprintf(stdout, "entrou aqui\n");

			add_to_history(input);

			struct commands *commands = parse_commands_with_pipes(input);
			 for (int i = 0; i < commands->cmd_count; i++)
			 {
			     fprintf(stdout, "command: ");
				 fprintf(stdout,"%s with %d", commands->cmds[i]->name,  commands->cmds[i]->argc);
			 }

			int historico_numero = 0;
			check_built_in(commands->cmds[0]);


			//caso seja historico 
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


			// if(commands->cmd_count > 1)
			// 	add_to_history(linecopy);

			// for (int i = 0; i < commands->cmd_count; i++)
			// {
			//     fprintf(stdout, commands->cmds[i]->name);
			//     fprintf(stdout, " n args: ");

			//     fprintf(stdout, "%d", commands->cmds[i]->argc);
			//     fprintf(stdout, "  >> \n");

			// }

			// struct command *command = parse(input);

			// for (int i = 0; i < command->argc; i++)
			// {
			// 	fprintf(stdout, command->argv[i]);
			// 	fprintf(stdout, " >> ");
			// }
		}
		else{
			
			exec_ret = exec_commands(commands);
		}	

		

		free(input);

		/* get ready to exit */
		if (exec_ret == -1)
			break;
	}

	
}

}