
#include <stdio.h>
#include <stdlib.h>

#include "../db_config.h"
#include "../dbms/file_manager/db_file.h"
#include "../dbms/buffer_manager/db_buffer.h"

int main(void) {
	start_buffer();
	free_list->print_function = buffer_print_page;

	list_print(free_list);

	//printf("\nPress Any Key to Continue\n");
	//getchar();
	return EXIT_SUCCESS;
}

