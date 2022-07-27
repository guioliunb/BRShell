#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <iostream>
#include "HelloShell.h"
#include <bits/stdc++.h>

static char **history = NULL; /* array of strings for storing history */
static int history_len = 0;
using namespace std;

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

	for (i = 0; i < n; i++)
	{
		if (!isspace(input[i]))
			return 0;
	}
	return 1;
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

	cmds = (commands *)calloc(sizeof(commands) + commandCount * sizeof(command *), 1);

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

	struct command *cmd = (command *)calloc(sizeof(struct command) + ARG_MAX_COUNT * sizeof(char *), 1);

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

int main(void)
{
	int exec_ret;
	char c;

	while (1)
	{
		fputs("BRsh - User->", stdout);

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

		if (strlen(input) > 0 && !is_blank(input) && input[0])
		{
			string linecopy = strdup(input);

			add_to_history(input);

			struct commands *commands = parse_commands_with_pipes(input);

			int historico_numero = 0;

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
					fprintf(stdout, history[i]);
					fprintf(stdout, "\n");
				}
				
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

		free(input);

		/* get ready to exit */
		if (exec_ret == -1)
			break;
	}

	return 0;
}