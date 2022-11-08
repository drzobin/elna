// Code by Robin Larsson aka drzobin
// robin.larsson@protonmail.ch
// 2022-11-07

#include "shared_funcs.h"
#include <stddef.h>
#include <stdio.h>

/*
Copy 0 to a int array, this function has a stack based overflow vulnerabilitie.  
@return void
*/
void vuln_func(int size){
	int buf[0];
	
	for(int i = 0; i < size; i = i +1){
		buf[i] = 0;
	}
}

int main(int argc, char *argv[]) {

	// Name of the file to read.
	char file[] = argv[1];

	// Data in the file to read.
	char *file_data;
	
	// Size of the file to read.
	size_t file_size = get_filesize(file);
	
	// Read data in file to heap.
	file_data = read_file(file,file_size);
	
	// Offset of data on heap to parse.
	int offset = 1;
	
	// Print offset value. 
	printf("offset: %d hex: %hhx\n",offset,file_data[offset]);
	
	// This function has a stack based overflow vulnerabilitie.
	vuln_func(file_data[offset]);
	
	return 0;
}
