#ifndef DATA_CONVERT_H_INCLUDED
#define DATA_CONVERT_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

void write_int(char *data, int number, unsigned long offset) {
	data[offset + 3] = (number >> 24) & 0xFF;
	data[offset + 2] = (number >> 16) & 0xFF;
	data[offset + 1] = (number >> 8)  & 0xFF;
	data[offset] = number & 0xFF;
}

int read_int(char *data, unsigned long offset) {
	char buffer[4];
	buffer[3] = data[offset + 3];
	buffer[2] = data[offset + 2];
	buffer[1] = data[offset + 1];
	buffer[0] = data[offset    ];
	return *(int*) buffer;
}

void write_string(char * data, char * string, int string_size, unsigned long offset){
	for(int i = 0; i < string_size; i++){
		data[offset + i] = string[i];
	}
}

void write_string(char * data, char * string, int string_size, unsigned long offset){
	for(int i = 0; i < string_size; i++){
		data[offset + i] = string[i];
	}
}

#endif
