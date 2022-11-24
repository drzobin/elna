CC=clang
CFLAG=-g -Wall -ansi -pedantic

all: elna crash_me

crash_me:
	$(CC) $(CFLAGS) crash_me.c shared_funcs.c -o crash_me

elna:
	$(CC) $(CFLAGS) elna.c shared_funcs.c -o elna
