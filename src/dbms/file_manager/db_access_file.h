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
#include <fcntl.h>     // To Open

#include <errno.h> // NEW


int file_open(char * path) {
	int file;
    
    #ifdef O_DIRECT_MODE
    	file = open(path, O_RDWR | O_CREAT | O_DIRECT, 0777); // O_SYNC ?
    #else
    	file = open(path, O_RDWR | O_CREAT, 0777);
    #endif

	return file;
}



void file_close(int file_descriptor){
	close(file_descriptor);
}

int file_size(int file_descriptor){
	return lseek(file_descriptor, 0, 2); //2 = SEEK_END
}



void file_read(int file_descriptor, long block_id, char * frame, int block_size){

	lseek(file_descriptor, block_id * block_size, 0); //0 = SEEK_SET
	if( read(file_descriptor, frame, block_size) == -1 ){
		printf("\n[ERR0] Read Page %d-%ld \n", file_descriptor, (size_t) block_id);
		perror("Error reading file");
		exit(1);
	}

}

void file_write(int file_descriptor, long block_id, char * frame, int block_size){
	lseek(file_descriptor, block_id * block_size, 0); //0 = SEEK_SET
	if( write(file_descriptor, frame, block_size) == -1){
		printf("\n[ERR0] Write Page %d-%ld \n",file_descriptor, (size_t) block_id);
		perror("Error writing file");
		exit(1);
	}

	#ifdef FSYNC_MODE
	if (fdatasync(file_descriptor) == -1) {
		perror("Error syncing file");
		exit(1);
	}
	#endif
}


#endif
