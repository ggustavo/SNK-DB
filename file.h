#ifndef FILE_ACCESS_H_INCLUDED
#define FILE_ACCESS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

FILE * open_file(char * path){
	FILE * file = fopen(path, "rb");
	if(file == NULL){
		//ERROR
        printf("\n ! FILE ERROR !");
	}
	return file;
}

int file_size(FILE *file){
    fseek(file, 0, SEEK_END); 
    return ftell(file); 
}

char * read_block(FILE *file, int block_id, int block_size){
	char * xs = (char*)malloc(block_size);
	fseek(file, block_id * block_size, SEEK_SET);
	fread(xs, sizeof(char),block_size ,file);
	return xs;
}

void write_block(FILE *file, int block_id, int block_size, char* data){
	fseek(file, block_id*block_size, SEEK_SET);
	fwrite(data, sizeof(char), block_size ,file);
	fflush(file);
}


void write_int(char * data, int number, int offset){
	data[offset + 3] = (number>>24) & 0xFF;
	data[offset + 2] = (number>>16) & 0xFF;
	data[offset + 1] = (number>>8) & 0xFF;
	data[offset    ] = number & 0xFF;
}


int read_int(char * data, int offset){
	char buffer[4];
	 buffer[3] = data[offset + 3];
	 buffer[2] = data[offset + 2];
	 buffer[1] = data[offset + 1];
	 buffer[0] = data[offset    ];
	return *(int *)buffer;
}



#endif
