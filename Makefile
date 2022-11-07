CC=clang
CFLAG=-g

all: test_binary

test_binary:
	$(CC) $(CFLAGS) test_binary.c shared_funcs.c -o test_binary
