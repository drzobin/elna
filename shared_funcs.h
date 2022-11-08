// Code by Robin Larsson aka drzobin
// robin.larsson@protonmail.ch
// 2022-11-07

#ifndef FUNKTION_H_INCLUDED
#define FUNKTION_H_INCLUDED
#include <stddef.h>

size_t get_filesize(const char* filename);

char *read_file(char *file,size_t size);

#endif
