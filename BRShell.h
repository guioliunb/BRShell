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

struct command *parse(char *input);
struct commands *parse_lote_com_pipes(char *input);
struct commands *parse_comandos_com_pipes(char *input);
char *ler_entrada(void);
int adicionar_na_historia(char *input);
void mostrar_ajuda();
char *sub_string(const char *src, int m, int n);
int comandos_internos(struct command *cmd);
int executar_comando(struct command *cmd);
void exec_async(char *argv[], int argc);
int verify_parsing(char *argv);
void exec_redirect(char *path, char *argv[], int mode);
void execution_piped(struct commands *cmds);
int executar_comandos(struct commands *cmds);
void usuario_diretorio();
void boas_vindas();
struct command *buscarAlias(char *input);
void lerAlias();

#endif
