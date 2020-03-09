/*
#include <stdio.h>
#include <stdlib.h>

#include "../db_config.h"
#include "../dbms/file_manager/db_file.h"
#include "../dbms/buffer_manager/db_buffer.h"

int main(void) {

	int file = file_open("Debug/test.txt");

	file_write(file,0,"FILE-00\n\0",BLOCK_SIZE);
	file_write(file,1,"FILE-01\n\0",BLOCK_SIZE);
	file_write(file,2,"FILE-02\n\0",BLOCK_SIZE);
	file_write(file,3,"FILE-03\n\0",BLOCK_SIZE);
	file_write(file,4,"FILE-04\n\0",BLOCK_SIZE);
	file_write(file,5,"FILE-05\n\0",BLOCK_SIZE);

	char * buffer = (char*)malloc(BLOCK_SIZE);

	file_read(file, 0, buffer, BLOCK_SIZE);
	printf("\n%s\n",buffer);

	file_read(file, 3, buffer, BLOCK_SIZE);
	printf("\n%s\n",buffer);

	file_read(file, 5, buffer, BLOCK_SIZE);
	printf("\n%s\n",buffer);

	file_close(file);

	return EXIT_SUCCESS;
}
*/
