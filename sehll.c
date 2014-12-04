#include "sehll.h"

#define MAX_LINE_SIZE 1024
#define MAX_ARGS 100
#define PROMPT ">#"

pid_t children[10];
int nchildren = 0;

int INPUT_FD = 0;
int OUTPUT_FD = 1;

int main() {
	char *line = malloc(2*MAX_LINE_SIZE * sizeof(char) + 1);
	const char **args = malloc(MAX_ARGS*2 * sizeof(char*));
	const char **files = malloc(21 * sizeof(char*));
	int reached_eof   = 0;
	while(!reached_eof) {
		printf("%s", PROMPT);
		reached_eof = read_line(line, MAX_LINE_SIZE + 1);
		if(*line == '\n') {
		    continue;
		}
		int num_args = parse(line, args);
		build_file_array(args, files, num_args);
		int index = 0;
		// while(*(files + index) != NULL) {
		//     printf("%s", *(files + index++));
		// }
		execute(*args, args, files);
	}
}

void fix_fds() {
    if(INPUT_FD != 0) {
	printf("Changing input fd to %d.\n", INPUT_FD);
	dup2(INPUT_FD, 0);
	close(INPUT_FD);
    }
    if(OUTPUT_FD != 1) {
	printf("Changing output fd to %d.\n", OUTPUT_FD);
	dup2(OUTPUT_FD, 1);
	close(OUTPUT_FD);
    }
}

int is_redir_char(int c) {
    return (c == '|' || c == '>' || c == '<' || c == '&');
}

void build_file_array(const char **args, const char **output, int num_args) {
    int i = 0;
    int c = 0;
    int found_null = 1;
    while(i < num_args) {
	if(found_null && *(args + i) != NULL) {
	    found_null = 0;
	    *(output + c++) = *(args + i);
	}
	else if(*(args + i) == NULL) {
	    found_null = 1;
	}
	i++;
    }
    *(output + i) = NULL;
}

void s_handler(int s_no) {
    if(s_no == SIGINT) {
	printf("Got kill sig.\n");
	if(nchildren == 0) {
	    kill(getppid(), s_no);
	}
	else {
	    for(int i=0; i<nchildren; i++) {
		kill(children[i], s_no);
	    }
	    nchildren = 0;
	}
    }
}

int execute(const char *file, const char **args, const char **files) {
	pid_t childProc = fork();
	int saved_stdout = dup(STDOUT_FILENO);
	int saved_stdin = dup(STDIN_FILENO);

	printf("Copied riginal stdin,stdout to: %d,%d\n", saved_stdin, saved_stdout);

	if(childProc >= 0) {
		if(childProc == 0) {
			children[nchildren++] = childProc;
			int i=0;
			fix_fds(files);
			execvp(file, args);
			printf("Error: %s\n", strerror(errno));
		}
		else {	
			if(signal(SIGINT, s_handler) == SIG_ERR) {
				printf("Error catching interrupt signal.\n");
			}
			int status;

			waitpid(childProc, &status, 0);

			dup2(1, saved_stdout);
			dup2(0, saved_stdin);

			OUTPUT_FD = 1;
			INPUT_FD = 0;

			close(saved_stdout);
			close(saved_stdin);

			return status;
		}
	}
	else {
		printf("Error: %s\n", strerror(errno));
	}
	return -1;
}

int read_line(char* line, int size) {
	int index = 0;
	int c;
	while((c=getchar()) != EOF && c != '\n' && index < size-1) {
		if(is_redir_char(c)) {
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
	 while((token=strtok(NULL, delim)) != NULL && word_index < MAX_ARGS) {
	 	// printf("Word %d: %s\n", word_index, token);
	 	// token = strtok(NULL, delim);
		switch(*token) {
		    case '>':
			OUTPUT_FD = open(strtok(NULL, delim), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
			break;
		    case '<':
			INPUT_FD = open(strtok(NULL, delim), O_RDONLY);
			break;
		    case '|':
			*(output + word_index++) = NULL;
		        *(output + word_index++) = token;
			*(output + word_index++) = NULL;
		    default:
			*(output + word_index++) = token;
		}
	 }
	 *(output + word_index) = NULL;
	 return word_index;
}
