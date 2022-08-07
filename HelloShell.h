#define ARG_MAX_COUNT    1024      /* max number of arguments to a command */
#define HISTORY_MAXITEMS 100 

struct command {		   /* a struct that stores a parsed command */
	int argc;                  /* number of arguments in the comand */
	char *name;                /* name of the command */
	char *argv[ARG_MAX_COUNT]; /* the arguments themselves */
	int fds[2];                /* the file descriptors for input/output */
};

struct commands {                  /* struct to store a command pipeline */
	int cmd_count;             /* number of commands in the pipeline */
	struct command *cmds[];    /* the commands themselves */
};

int verify_parsing(char *argv);
void exec_redirect(char *path, char *argv[], int mode);
void exec_async(char *argv[], int argc);
