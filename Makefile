# Code by Robin Larsson aka drzobin
# robin.larsson@protonmail.ch
# me@drz.se
# 2023-01-20

CC=clang
CFLAG=-g3 -Wall -Wextra -ansi -pedantic -fsanitizer=address,undefined

all: elna crash_me

crash_me:
	$(CC) $(CFLAGS) crash_me.c shared_funcs.c -o crash_me

elna:
	$(CC) $(CFLAGS) elna.c shared_funcs.c -o elna
