CC=clang
CFLAG=-g -Wall -ansi -pedantic

all: elna test_binary

test_binary:
	$(CC) $(CFLAGS) test_binary.c shared_funcs.c -o test_binary

elna:
	$(CC) $(CFLAGS) elna.c -o elna
