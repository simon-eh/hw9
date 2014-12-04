CC=gcc
CFLAGS=-std=c99 -g -Wall -Wextra -pedantic -D_POSIX_SOURCE

all: sehll
sehll: sehll.c
	$(CC) $(CFLAGS) sehll.c -o sehll
clean:
	rm -f sehll
