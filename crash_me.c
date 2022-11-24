// Code by Robin Larsson aka drzobin
// robin.larsson@protonmail.ch
// 2022-11-07

#include "shared_funcs.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
Copy 512 A from heap to char dst[0] on stack, this function has a stack based overflow vulnerabilitie.  
@return void
*/
void vuln_func(){
	
	char *src;
	char dst[0];
	src = (char *) malloc(512);
	
	memset(src,65,512);	
	memcpy(dst,src,strlen(src));

	printf("dst: %s \n",dst);
}

int main(int argc, char *argv[]) {

	// Name of the file to read.
	char *file = argv[1];

	// Data in the file to read.
	char *file_data;
	
	// Size of the file to read.
	size_t file_size = get_filesize(file);
	
	// Read data in file to heap.
	file_data = read_file(file,file_size);
	
	// Offset of data on heap to parse.
	int offset = 1;
	
	// Print offset value. 
	printf("offset: %d,data as hex: %hhx, data as int: %d\n",offset,file_data[offset],file_data[offset]);
	
	// This function has a stack based overflow vulnerabilitie.
	if(file_data[offset] == 65){
		vuln_func();
	}
	
	return 0;
}
