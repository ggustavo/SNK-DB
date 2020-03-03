#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>


struct String{
	char * value;
	int size;
};



struct String * string_create(char * value, int size){
	struct String * new_string = (struct String*) malloc (sizeof(struct String));
	new_string->size =size;
	new_string->value = value;
	return new_string;
}

void string_free(struct String * string){
	free(string->value);
	free(string);
	string = NULL;
}

struct String * string_concat(struct String * a, struct String * b){
	struct String * new_string = (struct String*) malloc (sizeof(struct String));
	new_string->size = a->size + b->size;
	new_string->value = (char*) malloc (new_string->size);

	for(int i = 0; i < a->size; i++){
		new_string->value[i] = a->value[i];
	}

	for(int i = 0; i < b->size; i++){
		new_string->value[i + a->size] = b->value[i];
	}

	return new_string;
}


struct String * string_concat_delimiter(struct String * a, char delimiter, struct String * b){
	struct String * new_string = (struct String*) malloc (sizeof(struct String));
	new_string->size = a->size + b->size + 1;
	new_string->value = (char*) malloc (new_string->size);

	for(int i = 0; i < a->size; i++){
		new_string->value[i] = a->value[i];
	}
	new_string->value[a->size] = delimiter;

	for(int i = 0; i < b->size; i++){
		new_string->value[i + a->size + 1] = b->value[i];
	}

	return new_string;
}






#endif
