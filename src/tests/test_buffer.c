
#include <stdio.h>
#include <stdlib.h>

#include "../dbms/db_kernel.h"
#include "../dbms/file_manager/data_handle.h"

int main(void) {
	start_database();

	//File Data: (with BLOCK_SIZE = 8 and BUFFER_SIZE = 5)
	//FILE--00FILE--XXFILE--02FILE--03FILE--04FILE--05FILE--06

	int file = file_open("Debug/test.txt");

	for(int i = 0; i < 5; i++){
		struct Page * p = buffer_request_page(file,i,WRITE_REQUEST);

		printf("\nMRU -> ");
		list_print(list);
		printf("<- LRU");
		write_string(p->data,"FILE--XX",BLOCK_SIZE,0);
		//buffer_print_page_complete(p);

	}

	printf("\nFree List -> ");
	list_print(free_list);


	struct Page * p = buffer_request_page(file,0,READ_REQUEST);
	printf("\nMRU -> ");
	list_print(list);
	printf("<- LRU");

	p = buffer_request_page(file,5,READ_REQUEST);
	printf("\nMRU -> ");
	list_print(list);
	printf("<- LRU");
	buffer_print_page_complete(p);

	p = buffer_request_page(file,5,WRITE_REQUEST);
	printf("\nMRU -> ");
	list_print(list);
	printf("<- LRU");
	buffer_print_page_complete(p);

	buffer_print_statistics();
	//printf("\nPress Any Key to Continue\n");
	//getchar();
	return 0;
}

