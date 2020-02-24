
#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#include <sys/file.h>  //to Open
#include <sys/types.h> //to Read and Write
#include <unistd.h>    //to Close


int file_open(char * path) {
	int file = open(path, 0x0100 | 2 , 0777); // O_CREAT | O_RDWR
	if (file == -1) {
		perror(path);
	}
	return file;
}

void file_close(int file_descriptor){
	close(file_descriptor);
}

void file_read(int file_descriptor, long block_id, char * frame, int block_size){
	lseek(file_descriptor, block_id * block_size, 0);
	read(file_descriptor, frame, block_size);
}

void file_write(int file_descriptor, long block_id, char * frame, int block_size){
	lseek(file_descriptor, block_id * block_size, 0);
	write(file_descriptor, frame, block_size);
}


#endif
