#include<sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include<stdio.h>
#include <string.h>

void lsh_loop(void);
char* lsh_read_line(void);
char **lsh_split_line(char*);
int lsh_launch(char**);
int lsh_execute(char**);

int main(int argc, char **argv) {
	// load config files, if any

	// run shell loop
	lsh_loop();

	// perform any shutdown / cleanup

	return EXIT_SUCCESS;
}

void lsh_loop(void) {
	char* line;
	char** args;
	int status;

	do {
		printf("$ ");
		line = lsh_read_line();// read input line(allow user input)
		args = lsh_split_line(line);// split the line into args
		status = lsh_execute(args);// execute the args
	} while (status);
}

#define LSH_RL_BUFSIZE 1024

// input from user. an array named buffer stores the user input

char* lsh_read_line(void) {
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char* buffer = malloc(sizeof(char) * bufsize);
	int c; // because EOF is integer so declare c as integer

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// Read a character
		c = getchar();

		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		}
		else {
			buffer[position] = c;
		}
		position += 1;

		if (position >= bufsize) {
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

// tokenize(separate) the user input into command and arguments

#define LSH_TOK_BIFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line) {
	int bufsize = LSH_TOK_BIFSIZE;
	int position = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token; // store and take one token at a time from line which is our buffer 

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	// takes line and separates words with delimiters
	token = strtok(line, LSH_TOK_DELIM);

	// add token one at a time to double pointer tokens
	while(token != NULL) {
		*(tokens + position) = token;
		position += 1;

		if (position >= bufsize) {
			bufsize += LSH_TOK_BIFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int lsh_launch(char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		// child process
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} else if(pid < 0) {
		perror("lsh");
	} else {
		// parent process
		do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

/*
Function declaration for builtin shell commands
*/

int lsh_cd(char**);
int lsh_help(char**);
int lsh_exit(char**);


char *builtin_str[] = {
	"cd",
	"help",
	"exit"

};

int (*builtin_func[]) (char **) = {
	&lsh_cd,
	&lsh_help,
	&lsh_exit 
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("Shatakshi's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}

int lsh_execute(char **args) {
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}
	return lsh_launch(args);
}