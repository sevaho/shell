//A shell does three main things:
//Initialize
//Interpret (reading commands)
//Terminate

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int sh_chd(char **args);
int sh_help(char **args);
int sh_exit(char **args);

char *builtin_str[] = {
   
	"chd",
	"help",
	"exit"

};

int (*builtin_func[]) (char **) = {

	&sh_chd,
	&sh_help,
	&sh_exit

};

int sh_num_builtins(){

	return sizeof(builtin_str) / sizeof(char *);

}

int sh_chd(char **args){

	if (args[1] == NULL){

		fprintf(stderr, "expected argument to \"chd\"\n");
	}else {

		if (chdir(args[1]) != 0){

			perror("sh");
		}
	}

return 1;

}

int sh_help(char **args){

	int i;
	printf("help");

	return 1;

}

int sh_exit(char **args){

	return 0;

}


int sh_launch(char **args){

	pid_t pid, wpid;
	int status;

	pid = fork();

	if (pid == 0){

		if (execvp(args[0], args) == -1){

			perror("sh");
		}

		exit(EXIT_FAILURE);
	} else if (pid < 0) {

		perror("sh");

	} else {

		do {

			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;

}

int sh_execute(char **args){

	int i;

	if (args[0] == NULL) {

		return 1;
	}

	for (i=0; i<sh_num_builtins(); i++){

		if (strcmp(args[0], builtin_str[i]) == 0){

			return (*builtin_func[i])(args);
		}
	}

	return sh_launch(args);
}

char *sh_read_line(void){

	int buffersize = 1024;
	int position = 0;
	char *buffer = malloc(sizeof(char) *buffersize);
	int c;

	if(!buffer){
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {

		c = getchar();

		if (c == EOF || c == '\n'){

			buffer[position] = '\0';
			return buffer;
		} else {

			buffer[position] = c;
		}
		position++;

		if (position >= buffersize){

			buffersize +- 1024;
			buffer = realloc(buffer, buffersize);

			if (!buffer){

				fprintf(stderr, "allocatation error \n");
				exit(EXIT_FAILURE);

			}
		}
	}
}

#define SH_TOK_DELIM " \t\r\n\a"
char **sh_split_line(char *line){

	int buffersize = 64, position = 0;
	char **tokens = malloc(buffersize * sizeof(char *));
	char *token;

	if (!tokens) {

		fprintf(stderr, "allocation error \n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, SH_TOK_DELIM);

	while (token != NULL){

		tokens[position] = token;
		position++;

		if (position >= buffersize){

			buffersize += SH_TOK_DELIM;
			tokens = realloc(tokens, buffersize * sizeof(char*));

			if (!tokens){

				fprintf(stderr, "allocation error \n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, SH_TOK_DELIM);
	}

	tokens[position] = NULL;
	return tokens;
}

#define KGRN  "\x1B[32m"
#define KWHT  "\x1B[37m"
#define KNRM  "\x1B[0m"
#define KCYN  "\x1B[36m"
void shell_loop(void){

	char *line;
	char **args;
	int status;

	do{
		uid_t uid;/*uid_t is a build in data type, which can hold small numbers*/
	   	uid=getuid();
		char cwd[1024];

   		if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, "◀{%s%d-%s%s%s}▶$ ",KGRN, uid, KCYN, cwd, KNRM);

		line = sh_read_line();
		args = sh_split_line(line);
		status = sh_execute(args);

		free(line);
		free(args);

	} while (status);
}

int main (int argc, char **argv){

	shell_loop();

	return EXIT_SUCCESS;
}
