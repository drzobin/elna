// Code by Robin Larsson aka drzobin
// robin.larsson@protonmail.ch
// 2022-11-07

#include "stdlib.h"
#include "stdio.h"
#include "sys/stat.h"

#include "shared_funcs.h"

/*
Get the size of a file.
@return The filesize, or 0 if the file does not exist.
*/
size_t get_filesize(const char* filename) {
    struct stat st;

    if(stat(filename, &st) != 0) {
        return 0;
    }

    return st.st_size;   
}


/*
Reade a file on disc and puts the content on heap.
@return Char pointer to content on heap.
*/
char *read_file(char *file,size_t size){
    // Ptr to file on heap
    char *ptr_file;

    // Ptr to file
    FILE *ptr_fp;

    // Allocate  memory on heap for file
    //ptr_file = (char *)malloc(size);
    ptr_file = (char *)malloc(size);


    // Open file for reading
    if((ptr_fp = fopen(file, "rb"))==NULL)
    {
        printf("Unable to open the file!\n");
        exit(1);
    }
    else 
    {
        printf("Opened file successfully for reading.\n");
    }

    // Reading file into memory
    if(fread(ptr_file, size, 1, ptr_fp) != 1)
    {
        printf( "Read error!\n" );
	exit( 1 );
    }
    else 
    {
        printf( "Read was successful.\n" );
    }
    fclose(ptr_fp);


    for(int i = 0;i < size; i++){
        printf("pos: %d hex: %hhx\n",i,ptr_file[i]);
    }

    return ptr_file;
}

