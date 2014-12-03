CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic

all: sehll
sehll: sehll.c
	$(CC) $(CFLAGS) sehll.c -o sehll
clean:
	rm -f sehll
