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
	int *cmds = malloc(21 * sizeof(int));
	int reached_eof = 0;
	if(signal(SIGINT, s_handler) == SIG_ERR) {
	    printf("Error catching interrupt signal.\n");
	}
	if(signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
	    printf("Error catching child signal.\n");
	}
	while(!reached_eof) {
		printf("%s", PROMPT);
		reached_eof = read_line(line, MAX_LINE_SIZE + 1);
		if(*line == '\n') {
		    continue;
		}
		int num_cmds = parse(line, args, cmds);
		if(num_cmds == 1) {
		    execute(*args, args);
		}
		else {
		    run_cmd(args, cmds, num_cmds);
		}
	}
}

void fix_fds() {
    if(INPUT_FD != STDIN_FILENO) {
	printf("Changing input fd to %d.\n", INPUT_FD);
	dup2(INPUT_FD, STDIN_FILENO);
	close(INPUT_FD);
    }
    if(OUTPUT_FD != STDOUT_FILENO) {
	printf("Changing output fd to %d.\n", OUTPUT_FD);
	dup2(OUTPUT_FD, STDOUT_FILENO);
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
	// printf("Got kill sig.\n");
	// printf("Parent process id: %d\n", getppid());
	// printf("Children %d\n", nchildren);
	if(nchildren == 0) {
	    
	}
	else {
	    for(int i=0; i<nchildren; i++) {
		//printf("Killing %d\n", children[i]);
		kill(children[i], s_no);
	    }
	    nchildren = 0;
	}
    }
}

void run_cmd(const char **args, int *cmds, int numcmds) {
    int s_stdin = dup(STDIN_FILENO);
    int s_stdout = dup(STDOUT_FILENO);
    fix_fds();
    if(numcmds <= 0) {
	return;
    }

    int fds[2*(numcmds-1)];
    for(int i=0; i<numcmds-1; i++) {
	if(pipe(fds + i*2) < 0) {
	    fprintf(stderr, "Pipe error on line %d", __LINE__);
	    perror("");
	    restore_fds(s_stdin, s_stdout);
	    return;
	}
    }
    int childpid;
    for(int i=0; i<numcmds; i++) {
	int index = cmds[i];

	childpid = fork();
	if(childpid == 0) {
	    // child proc
	    fprintf(stderr, "%d\n", i);
	    if(i < numcmds-1) {
		fprintf(stderr, "Duping stdout for cmd %s\n", *(args + index));
		if(dup2(fds[i*2 + 1], 1) < 0) {
		    fprintf(stderr, "Dup2 error on line %d", __LINE__);
		    perror("");
		    // restore_fds(s_stdin, s_stdout);
		    exit(0);
		}
	    }
	    if(i > 0) {
		fprintf(stderr, "Duping stdin for cmd %s\n", *(args + index));
		if(dup2(fds[i*2-2], 0) < 0) {
		    fprintf(stderr, "Dup2 error on line %d\n", __LINE__);
		    fprintf(stderr, "File descriptor: %d", fds[i*2-2]);
		    perror("");
		    // restore_fds(s_stdin, s_stdout);
		    // exit(0);
		    kill(getppid(), SIGINT);
		    exit(0);
		}
	    }
	    for(int j=0; j<numcmds*2-2; j++) {
		close(fds[j]);
	    }
	    fprintf(stderr, "Exec cmd %s\n", *(args + index));
	    execvp(*(args + index), args + index);
	    exit(0);
	}
	else if(childpid < 0) {
	    fprintf(stderr, "Fork Error on line %d", __LINE__);
	    perror("");
	    // restore_fds(s_stdin, s_stdout);
	    exit(0);
	}
	else {
	    children[nchildren++] = childpid;
	}
    }
    for(int i=0; i<2*numcmds-1; i++) {
	close(fds[i]);
    }
    int status;
    for(int i=0; i<numcmds; i++) {
	wait(&status);
    }
    restore_fds(s_stdin, s_stdout);
}

void restore_fds(int saved_stdin, int saved_stdout) {
    dup2(saved_stdin, 0);
    dup2(saved_stdout, 1);
    if(INPUT_FD != 0) {
	close(INPUT_FD);
    }
    if(OUTPUT_FD != 1) {
	close(OUTPUT_FD);
    }
    INPUT_FD = 0;
    OUTPUT_FD = 1;
}

int execute(const char *file, const char **args) {
	pid_t childProc = fork();
	int saved_stdout = dup(STDOUT_FILENO);
	int saved_stdin = dup(STDIN_FILENO);


	if(childProc >= 0) {
		if(childProc == 0) {
			fix_fds();
			execvp(file, args);
			fprintf(stderr, "Error: %s\n", strerror(errno));
			exit(0);
		}
		else {	
			children[nchildren++] = childProc;
			int status;

			waitpid(childProc, &status, 0);

			restore_fds(saved_stdin, saved_stdout);

			return status;
		}
	}
	else {
		fprintf(stderr, "Error: %s\n", strerror(errno));
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

int parse(char *input, const char **output, int *cmd_pointers) {
	int word_index = 0;
	char delim[2] = " ";
	int num_cmds = 0;
	char *token = strtok(input, delim);
	if(token != NULL) {
	    num_cmds = 1;
	}
	*(output + word_index++) = token;
	*(cmd_pointers) = 0;
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
			*(output + word_index) = strtok(NULL, delim);
			*(cmd_pointers + num_cmds++) = word_index++;
			// printf("cmd %d: %s\n", num_cmds-1, *(output + cmd_pointers[num_cmds-1]));
			break;
		    default:
			// printf("Writing at index %d: %s\n", word_index, token);
			*(output + word_index++) = token;
		}
	 }
	 *(output + word_index) = NULL;
	 return num_cmds;
}
