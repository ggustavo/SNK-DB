/*
 * This file contains simple functions to access data on the secondary storage media.
 * These functions make system calls and are expected to be very fast.
 */
#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#include <sys/file.h>  // To Open
#include <sys/types.h> // To Read and Write
#include <unistd.h>    // To Close


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
	if( read(file_descriptor, frame, block_size) == -1 ){
		printf("\n[ERR0] Read Page %d-%ld",file_descriptor,block_id);
	}
}

void file_write(int file_descriptor, long block_id, char * frame, int block_size){
	lseek(file_descriptor, block_id * block_size, 0);
	if( write(file_descriptor, frame, block_size) == -1){
		printf("\n[ERR0] Write Page %d-%ld",file_descriptor,block_id);
	}
}


#endif
