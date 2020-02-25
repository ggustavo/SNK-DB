
#include <stdio.h>
#include <stdlib.h>

#include "../db_config.h"
#include "../dbms/file_manager/db_file.h"
#include "../dbms/buffer_manager/db_buffer.h"

int main(void) {
	buffer_start();


	int file = file_open("Debug/test.txt");
	printf("\nFile %d open",file);


	//file_read(file, 0, page->data, BLOCK_SIZE);
	//printf("\n%s\n",page->data);




	//printf("\nPress Any Key to Continue\n");
	//getchar();
	return EXIT_SUCCESS;
}

