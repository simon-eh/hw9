#include "sehll.h"

#define MAX_LINE_SIZE 1024
#define MAX_ARGS 100
#define PROMPT ">$"

int main() {
	char *line = malloc(2*MAX_LINE_SIZE * sizeof(char) + 1);
	const char **args = malloc(MAX_ARGS * sizeof(char*));
	int reached_eof   = 0;
	while(!reached_eof) {
		printf("%s", PROMPT);
		reached_eof = read_line(line, MAX_LINE_SIZE + 1);
		int num_args = parse(line, args);
		execute(*args,args);
		//printf("%d args\n", num_args);
		//if(num_args == 3) {
			//printf("Executing cmd %s\n", *args);
			//printf("Args: %s\n", *(args + 1));
			//execute(*args, args);
			// execvp(*args, args+1);
			// printf("Error: %s\n", strerror(errno));
			// execute single cmd
		//}
	}
}

void s_handler(int s_no) {
    if(s_no == SIGINT) {
	//kill((pid_t)(-1), s_no);
	printf("Got kill sig.\n");
    }
}

int execute(const char *file, const char **args) {
	pid_t childProc = fork();
	if(childProc >= 0) {
		if(childProc == 0) {
			if(signal(SIGINT, s_handler) == SIG_ERR) {
				printf("Error catching interrupt signal.\n");
			}
			int i=0;
			while(i++ < 1000000=00){}
			execvp(file, args);
			printf("Error: %s\n", strerror(errno));
		}
		else {
			int status;
			waitpid(childProc, &status, 0);
			return status;
		}
	}
	else {
		printf("Error: %s\n", strerror(errno));
	}
}

int read_line(char* line, int size) {
	int index = 0;
	int c;
	while((c=getchar()) != EOF && c != '\n' && index < size-1) {
		if(c == '|' || c == '>' || c == '<' || c == '&') {
			if(index > 0 && *(line + index - 1) != ' ') {
				*(line + index++) = ' ';
			}
			*(line + index++) = c;
			*(line + index++) = ' ';
		}
		else {
			*(line + index++) = c;
		}
	}
	// *(line + index) = '\n';
	*(line + index) = '\0';
	return (c == EOF);
}

int parse(char *input, const char **output) {
	int word_index = 0;
	char delim[2] = " ";
	 char *token = strtok(input, delim);
	 *(output + word_index++) = token;
	 while(token != NULL && word_index < MAX_ARGS) {
	 	// printf("Word %d: %s\n", word_index, token);
	 	token = strtok(NULL, delim);
	 	*(output + word_index++) = token;
	 }
	 *(output + word_index) = NULL;
	 return word_index;
}
