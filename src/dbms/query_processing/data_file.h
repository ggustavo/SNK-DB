#ifndef DATA_FILE_H_INCLUDED
#define DATA_FILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "../file_manager/db_file.h"
#include "../buffer_manager/policies/LRU.h"

struct DataFile{
	char *  path;
	int     file_id;
	char *  name;
	int number_of_blocks;
	int number_of_tuples;
};

struct DataFile * data_file_open(char * path){
	int file = file_open(path);
	struct DataFile * data_file = (struct DataFile *) malloc (sizeof(struct DataFile));
	data_file->path = path;
	data_file->file_id = file;
	return data_file;
}

struct DataFile * data_file_fill_meta(struct DataFile * data_file){
	return data_file;
}


#endif
