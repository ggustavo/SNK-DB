#ifndef DATA_FILE_H_INCLUDED
#define DATA_FILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "../buffer_manager/policies/LRU.h"
#include "../file_manager/block_header.h"
#include "../file_manager/db_access_file.h"
#include "../file_manager/data_handle.h"

struct DataFile{
	char *  path;
	int     file_id;
	char *  name;
	int number_of_blocks;
	int number_of_tuples;
};

struct DataFile * data_file_open(char * name, char * path){
	int file = file_open(path);
	struct DataFile * data_file = (struct DataFile *) malloc (sizeof(struct DataFile));
	data_file->path = path;
	data_file->file_id = file;
	data_file->name = name;
	data_file->number_of_blocks = 0;
	data_file->number_of_tuples = 0;
	return data_file;
}

struct DataFile * data_file_fill_meta(struct DataFile * data_file){
	return data_file;
}


int data_file_write_tuple_in_block(struct BlockHeader * header, char * data, char * tuple, int tuple_size){

	int free_space = BLOCK_SIZE - ( HEADER_SIZE + ( header->tuple_index_size * INDEX_ENTITY_BYTES) + header->used_size );

	if(tuple_size + INDEX_ENTITY_BYTES > free_space){
		return ERROR_BLOCK_IS_FULL;
	}

	int new_index_entity = 0;

	if(header->tuple_index_size == 0){ // We have the First Tuple!

		new_index_entity = BLOCK_SIZE - tuple_size;

		handler_write_int(data, new_index_entity, OFFSET_FIRST_ENTITY_INDEX); // Write the entity in the first position of the tuples index


	}else{ // There is already at least one tuple in the index ( header->tuple_index_size > 0 )

		int last_index_entidy = handler_read_int(data, OFFSET_FIRST_ENTITY_INDEX + ( ( header->tuple_index_size - 1 ) * INDEX_ENTITY_BYTES ) );

		new_index_entity = last_index_entidy - tuple_size;

		handler_write_int(data, new_index_entity,  OFFSET_FIRST_ENTITY_INDEX + ( header->tuple_index_size * INDEX_ENTITY_BYTES ) ); // Write the entity in the last position of the tuples index

	}

	header->tuple_index_size = header->tuple_index_size + 1;
	header->used_size = header->used_size + tuple_size;

	handler_write_string(data, tuple, tuple_size, new_index_entity);             // Write the tuple in the space range [index_entity to (index_entity + tuple_size)]

	handler_write_int(data, header->tuple_index_size, OFFSET_TUPLES_INDEX_SIZE); // Write and updates the index size
	handler_write_int(data, header->used_size,        OFFSET_USED_SIZE);         // Write and updates the tuple used size

	return SUCCESS_WRITE_TUPLE;
}

void data_file_read_all_tuples_in_block(struct BlockHeader * header, char * data, void (*export_function)(struct BlockHeader*, int, char*, int)){
	char * tuple = NULL;
	int index_to_tuple = 0;
	int index_last_tuple = BLOCK_SIZE;

	for(int i = 0; i < header->tuple_index_size; i++){
		index_to_tuple = handler_read_int(data, OFFSET_FIRST_ENTITY_INDEX + ( i * INDEX_ENTITY_BYTES ));
		tuple = handler_read_string(data, index_last_tuple - index_to_tuple, index_to_tuple);
		export_function(header, i, tuple, index_last_tuple - index_to_tuple);
		index_last_tuple = index_to_tuple;
	}
}


#endif
