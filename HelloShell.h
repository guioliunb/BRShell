#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#define ARG_MAX_COUNT    1024
#define HISTORY_MAXITEMS 100 

struct command {		   
	int argc;            
	char *name;          
	char *argv[ARG_MAX_COUNT]; 
	int fds[2];          
};

struct commands {            
	int cmd_count;       
	struct command *cmds[];    
};

struct key_value
{
   int key;
   char* alias;
   char* value;
};
int historico(char *input);
int is_blank(char *input);

struct commands *parse_batch_with_pipes(char *input);
struct commands *parse_commands_with_pipes(char *input);
struct command *parse(char *input);
int add_to_history(char *input);
char *read_input(void);
void showHelp();
int verify_parsing(char *argv);
void exec_redirect(char *path, char *argv[], int mode);
int exec_command(struct command *cmd);
char* substr(const char *src, int m, int n);
int check_built_in(struct command *cmd);
void exec_async(char *argv[], int argc);
struct command *parse(char *input);
struct command *buscarAlias(char *input);
void exec_async(char *argv[], int argc);
void readAlias();
int verify_parsing(char *argv);
void exec_redirect(char *path, char *argv[], int mode);
void shellPrompt();
void execution_piped(struct commands *cmds);
void welcomeScreen();
int exec_commands(struct commands *cmds);



#endif
