/*
* The data file is an abstraction of the system 
* to access and manipulate data with physical organization in blocks
*/
#ifndef DATA_FILE_H_INCLUDED
#define DATA_FILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../file_manager/block_header.h"
#include "../file_manager/db_access_file.h"
#include "../file_manager/data_handle.h"

struct DataFile{
	int     file_id;
	int     meta_file_id;
	char *  name;

	char * directory_path;
	char * file_path;
	char * meta_file_path;


	int number_of_blocks;
	int number_of_tuples;
};


void data_file_print(struct DataFile * data_file){
	printf("\n---------------------------------------------------------------------------------------------------");
	printf("\nData File - Name: %s", data_file->name);
	printf("\nData File - Directory: %s", data_file->directory_path);
	printf("\nData File - Number of Blocks: %d", data_file->number_of_blocks);
	printf("\n---------------------------------------------------------------------------------------------------");
}

struct DataFile * data_file_load_meta(struct DataFile * data_file){ //TODO <---

	data_file->number_of_blocks = file_size(data_file->file_id) / BLOCK_SIZE;

	return data_file;
}


struct DataFile * data_file_create_meta(struct DataFile * data_file){ //TODO <---
	return data_file;
}


struct DataFile * data_file_open(char * name, char * path){
	char * directory_path = catalog_append_path(3, path, "/", name );
	char * file_path      = catalog_append_path(6, path, "/", name , "/", name, ".data" );
	char * meta_file_path = catalog_append_path(6, path, "/", name , "/", name, ".meta" );

	int result = catalog_access_directory(directory_path);

	int file_id = file_open(file_path);
	int meta_file_id = file_open(meta_file_path);

	struct DataFile * data_file = (struct DataFile *) malloc (sizeof(struct DataFile));

	data_file->file_id = file_id;
	data_file->meta_file_id = meta_file_id;
	data_file->name = name;

	data_file->directory_path = directory_path;
	data_file->file_path = file_path;
	data_file->meta_file_path = meta_file_path;

	data_file->number_of_blocks = 0;
	data_file->number_of_tuples = 0;

	if(result == OPENED){
		data_file_load_meta(data_file);

	} else if(result == CREATED){
		data_file_create_meta(data_file);

	}else{
		printf("\n[ERR0] Access Data File");

	}
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



int data_file_write_new_tuple(struct DataFile * data_file, char * tuple, int tuple_size){

	struct BlockHeader * temp_header = header_create();
	struct Page * page = NULL;

	if(data_file->number_of_blocks == 0){ // create a new block

		temp_header->block_id = 0;

		page = buffer_request_page(data_file->file_id, temp_header->block_id, WRITE_REQUEST);

		header_write_to_page(temp_header, page->data);

		data_file_write_tuple_in_block(temp_header, page->data, tuple, tuple_size); //TODO what to do when a tuple exceeds the block size?

		data_file->number_of_blocks = 1;

	}else {

		temp_header->block_id = data_file->number_of_blocks - 1; // last block written
		page = buffer_request_page(data_file->file_id, temp_header->block_id, WRITE_REQUEST);

		header_read_from_page(temp_header, page->data);

		int result = data_file_write_tuple_in_block(temp_header, page->data, tuple, tuple_size);

		if(result == ERROR_BLOCK_IS_FULL){
			//printf("\nBLOCK FULL!");
			data_file->number_of_blocks  = data_file->number_of_blocks + 1; // increment to create a new block

			header_reset(temp_header);
			temp_header->block_id = data_file->number_of_blocks - 1;

			page = buffer_request_page(data_file->file_id, temp_header->block_id, WRITE_REQUEST);

			header_write_to_page(temp_header, page->data);
			data_file_write_tuple_in_block(temp_header, page->data, tuple, tuple_size);

		}

	}

	header_free(temp_header);
	return SUCCESS_WRITE_TUPLE;
}

/*
char * read_tuple(struct DataFile * data_file, int block_id, int tuple_id){

	return NULL;
}

int data_file_write_update_tuple(){ //TODO
	return SUCCESS_WRITE_TUPLE;
}

int data_file_write_delete_tuple(){ //TODO

	return SUCCESS_WRITE_TUPLE;
}
*/

void data_file_read_all_tuples_in_block_e(struct BlockHeader * header, char * data, void (*export_function)(struct BlockHeader*, int, char*, int)){
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


void data_file_scan(struct DataFile * data_file, void (*export_function)(struct BlockHeader*, int, char*, int)){

	struct BlockHeader * temp_header = header_create();
	struct Page * page = NULL;

	for(int i = 0; i < data_file->number_of_blocks; i++){
		page = buffer_request_page(data_file->file_id, i, READ_REQUEST);
		header_read_from_page(temp_header, page->data);
		data_file_read_all_tuples_in_block_e(temp_header, page->data, export_function);


	}
	header_free(temp_header);
}

#endif
