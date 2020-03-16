/*
 * This file contains functions to write and read data from a page (char *).
 * Currently only three types:
 *
 * INT    ->           [4 bytes]
 * FLOAT  ->           [4 bytes]
 * String -> [string_size bytes]
 */

#ifndef DATA_HANDLE_H_INCLUDED
#define DATA_HANDLE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ------- INT ------------------------------------------------------
//###################################################################
void handler_write_int(char * data, int number, unsigned long offset) {
	data[offset + 3] = (number >> 24) & 0xFF;
	data[offset + 2] = (number >> 16) & 0xFF;
	data[offset + 1] = (number >> 8)  & 0xFF;
	data[offset]     = (number)       & 0xFF;
}

int handler_read_int(char * data, unsigned long offset) {
	char buffer[4];
	buffer[3] = data[offset + 3];
	buffer[2] = data[offset + 2];
	buffer[1] = data[offset + 1];
	buffer[0] = data[offset    ];
	return *(int*) buffer;
}
//###################################################################
// ------------------------------------------------------------------


// ------- FLOAT ----------------------------------------------------
//###################################################################
void handler_write_float(char * data, float number, unsigned long offset) {
	unsigned int asInt = *((int*)&number);
	data[offset + 3] = (asInt >> 24) & 0xFF;
	data[offset + 2] = (asInt >> 16) & 0xFF;
	data[offset + 1] = (asInt >> 8)  & 0xFF;
	data[offset] =     (asInt)       & 0xFF;
}

float handler_read_float(char * data, unsigned long offset) {
	char buffer[4];
	buffer[3] = data[offset + 3];
	buffer[2] = data[offset + 2];
	buffer[1] = data[offset + 1];
	buffer[0] = data[offset    ];
	return *(float*) buffer;
}
//###################################################################
// ------------------------------------------------------------------


// ------- STRING ---------------------------------------------------
//###################################################################
void handler_write_string(char * data, char * string, int string_size, unsigned long offset){
	memcpy(data + offset, string, string_size);
	//for(int i = 0; i < string_size; i++){ // <---------- look for a more efficient method
	//	data[offset + i] = string[i];
	//}
}

char * handler_read_string(char * data, int string_size, unsigned long offset){
	char * string = (char *) malloc(sizeof(string_size));
	memcpy(string, data + offset, string_size);
	//for(int i = 0; i < string_size; i++){ // <---------- look for a more efficient method
		//string[i] = data[offset + i];
	//}
	return string;
}
//###################################################################
// ------------------------------------------------------------------

#endif
