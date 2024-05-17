/*
#include <stdio.h>
#include <stdlib.h>

#include "../dbms/db_kernel.h"
#include "../dbms/file_manager/data_file.h"

void print_tuples(struct BlockHeader * header, int tuple_id, char * tuple, int tuple_size){
	printf("\n block_id=[%d] tuple_id=[%d] size=[%d] data=[%s]", header->block_id, tuple_id, tuple_size, tuple);
}



int main(void) {

	start_database();


	struct DataFile * data_file = data_file_open("users", CATALOG_DATA_FOLDER);
	data_file_print(data_file);


	data_file_write_new_tuple( data_file, "Tuple-00\0", 9);
	data_file_write_new_tuple( data_file, "Tuple-01\0", 9);
	data_file_write_new_tuple( data_file, "Tuple-02\0", 9);
	data_file_write_new_tuple( data_file, "Tuple-03\0", 9);
	data_file_write_new_tuple( data_file, "Tuple-04\0", 9);
	data_file_write_new_tuple( data_file, "Tuple-05\0", 9);

	data_file_write_new_tuple( data_file, "Tuple-06\0", 9); //<--- block full!


	data_file_scan(data_file, print_tuples);


	buffer_flush();
	buffer_print_statistics();

	//printf("\nPress Any Key to Exit\n");
	//getchar();
	return 0;
}

*/