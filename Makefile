CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic

all: sehll
sehll: sehll.c
	$(CC) $(CFLAGS) sehll.c -o sehll
run: sehll.exe
	./sehll.exe < test
clean:
	rm -f sehll.exe