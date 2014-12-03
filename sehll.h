#ifndef SEHLL_H
#define SEHLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int read_line(char*,int);
int parse(char*,const char**);
int execute(char*, const char**);

#endif