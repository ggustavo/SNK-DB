#ifndef CATALOG_H_INCLUDED
#define CATALOG_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_config.h"
#include "../util/db_strings.h"


#define CATALOG_DATABASE_FOLDER "databases"
#define CATALOG_SCHEMAS_FOLDER "schemas"



int catalog_open_directory(char * path){
	int dir = mkdir(path); //mkdir(path, 0777);
	if(dir >= 0 ) printf("\nCatalog - %s Directory Opened", path );
	return dir;
}


void catalog_start(){

	catalog_open_directory(CATALOG_DATABASE_FOLDER);
	catalog_open_directory( 	string_concat_delimiter(
									string_create(CATALOG_DATABASE_FOLDER, 9),
									'/',
									string_create(CATALOG_SCHEMAS_FOLDER, 7)
									)->value
			);



	printf("\n---------------------------------------------------------------------------------------------------");
}


#endif
