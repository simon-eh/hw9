#ifndef SEHLL_H
#define SEHLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>


void s_handler(int);
int read_line(char*,int);
int parse(char*,const char**);
int execute(const char*, const char**, const char**);
void build_file_array(const char**,const char**,int);
void fix_fds();
int is_redir_char(int);


#endif
