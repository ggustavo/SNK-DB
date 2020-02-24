/*
#include <stdio.h>
#include <stdlib.h>

#include "../db_config.h"
#include "../dbms/file_manager/db_file.h"
#include "../dbms/buffer_manager/db_buffer.h"

int main(void) {
	start_buffer();

	int id_array = 10;

	struct Page *page2 = &pages[1];
	char *content = page2->data;

	printf("\n-> %d", allocated_memory[id_array]);
	printf("\n-> %d", content[0]);

	allocated_memory[id_array] = 100;

	printf("\n-> %d", allocated_memory[id_array]);
	printf("\n-> %d", content[0]);

	content[0] = 102;

	printf("\n-> %d", allocated_memory[id_array]);
	printf("\n-> %d", content[0]);

	return EXIT_SUCCESS;
}
*/
