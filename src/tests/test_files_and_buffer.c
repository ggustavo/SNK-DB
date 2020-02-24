
#include <stdio.h>
#include <stdlib.h>

#include "../db_config.h"
#include "../dbms/file_manager/db_file.h"
#include "../dbms/buffer_manager/db_buffer.h"

int main(void) {
	start_buffer();



	int file = file_open("Debug/test.txt");
	printf("\nFile %d open",file);

	struct Node * free = free_list->head;
	struct Page * page = free->data;
	file_read(file, 1, page->data, BLOCK_SIZE);
	printf("\n%s\n",page->data);

	//find_page(1,2);
	free_list->print_function = buffer_print_page;
	//list_print(free_list);

	//printf("\nPress Any Key to Continue\n");
	//getchar();
	return EXIT_SUCCESS;
}

