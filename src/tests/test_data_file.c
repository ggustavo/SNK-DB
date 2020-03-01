
#include <stdio.h>
#include <stdlib.h>

#include "../dbms/db_kernel.h"
#include "../dbms/query_processing/data_file.h"
#include "../dbms/file_manager/block_header.h"


void export(struct BlockHeader * header, int index, char * tuple, int tuple_size){
	printf("\n ----> %d :: %s", tuple_size, tuple);
}


int main(void) {
	start_database();

	//File Data: (with BLOCK_SIZE = 8 and BUFFER_SIZE = 5)
	//FILE--00FILE--XXFILE--02FILE--03FILE--04FILE--05FILE--06


	struct DataFile * data_file = data_file_open("users","Debug/users.txt");
	struct BlockHeader * block = header_create();
	block->block_id = 0;

	struct Page * page = buffer_request_page(data_file->file_id, block->block_id, WRITE_REQUEST);
	header_write(block, page->data);


	//buffer_print_page_complete(page);
	header_print(block);


	data_file_write_tuple_in_block(block, page->data, "Tuple-00", 8);
	data_file_write_tuple_in_block(block, page->data, "Tuple-01", 8);
	data_file_write_tuple_in_block(block, page->data, "Tuple-02", 8);
	data_file_write_tuple_in_block(block, page->data, "Tuple-03", 8);
	data_file_write_tuple_in_block(block, page->data, "Tuple-04", 8);
	data_file_write_tuple_in_block(block, page->data, "Tuple-05", 8);
	data_file_write_tuple_in_block(block, page->data, "Tuple-06", 8); //BLOCK FULL HERE

	header_print(block);

	data_file_read_all_tuples_in_block(block, page->data, export);

	buffer_flush_page(page);

	//buffer_print_statistics();

	//printf("\nPress Any Key to Continue\n");
	//getchar();
	return 0;
}

