#ifndef CATALOG_H_INCLUDED
#define CATALOG_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h> //for use va_list
#include "../db_config.h"
#include <sys/stat.h>   //for use mkdi?
#include <sys/types.h> //for use mkdi?
#include <io.h>       //for use mkdir?


int catalog_access_directory(char * path){
	int dir = mkdir(path); //mkdir(path, 0777); S_IRWXU
	if(dir == 0 ){
		printf("\nCatalog - [CREATE] %s", path );
		return CREATED;
	}else{
		printf("\nCatalog - [OPEN] %s", path );
		return OPENED;
	}
	return dir;
}


void catalog_start(){


	catalog_access_directory(CATALOG_DATA_FOLDER);
	catalog_access_directory(CATALOG_SCHEMAS_FOLDER);

	/*
	catalog_open_directory( 	string_concat_delimiter(
									string_create(CATALOG_DATABASE_FOLDER, 9),
									'/',
									string_create(CATALOG_SCHEMAS_FOLDER, 7)
									)->value
			);
	 */


	printf("\n---------------------------------------------------------------------------------------------------");
}



char * catalog_append_path(int size, ...){
	va_list valist;
	int path_buffer_size = 0;
	int i = 0;
	char * path = NULL;

	va_start(valist, size); //start va_list arguments

	for (i = 0; i < size; i++) {
	    path = va_arg(valist, char*);
		path_buffer_size += strlen(path);
	}

	va_end(valist); //clean valist from memory

	va_start(valist, size); //start again va_list arguments

	char * path_buffer = calloc(path_buffer_size, sizeof(char));

	for (i = 0; i < size; i++) {
		 path = va_arg(valist, char*);
		 strcat(path_buffer,path);
	}


	va_end(valist); //clean valist from memory

	return path_buffer;
}

#endif
