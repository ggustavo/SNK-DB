
#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../util/doubly_linked_list.h"


#define STATUS_FREE 'F';
#define STATUS_BLOCKED 'B';

#define CLEAN 'C'
#define DIRTY 'D'


unsigned long long miss_ration = 0;
unsigned long long hit_ration = 0;
unsigned long long operations = 0;

struct Page{

	int file_id;
	int block_id;

	int frame_id;
	char * data; //pointer to allocated_memory frame
	char status;
	char dirty_flag;
};


char * allocated_memory;
struct Page * pages;
struct List * free_list;

void start_buffer() {
	allocated_memory = (char*) malloc( BUFFER_SIZE * BLOCK_SIZE);
	pages = (struct Page*) malloc(sizeof(struct Page) * BUFFER_SIZE);
	free_list = list_create(NULL,NULL);

	for (int i = 0; i < BUFFER_SIZE * BLOCK_SIZE; i++) {
		allocated_memory[i] = 0; //initialize the data allocated memory with 0

		if (i % BLOCK_SIZE == 0) {
			//printf("\nA: %d, D:%d", i, i / BLOCK_SIZE);
			struct Page *page = &pages[i / BLOCK_SIZE];
			page->frame_id = i / BLOCK_SIZE;
			page->block_id = 0;
			page->file_id = 0;
			page->dirty_flag = CLEAN;
			page->status = STATUS_FREE;
			page->data = &allocated_memory[i];
			list_insert_tail(free_list,page);
		}

	}
	printf("\n---------------------------------------------------------------------------------------------------");
	//printf("\nBuffer - Size of C Pointers: %d bytes", sizeof(void*));
	printf("\nBuffer - Number of Pages: %d", BUFFER_SIZE);
	printf("\nBuffer - Size of Block: %d bytes", BLOCK_SIZE);
	printf("\nBuffer - Size of Page (without data): %d bytes",sizeof(struct Page));
	printf("\nBuffer - Size of Free List Node: %d bytes",sizeof(struct Node));
	printf("\nBuffer - Data Alocated Memory ( Size of Block * Number of Pages ): %d bytes",(BUFFER_SIZE * BLOCK_SIZE));
	printf("\nBuffer - Total ( [Size of Page + Size of Block + Free List Node] * Number of Pages ): %d bytes",
			(sizeof(struct Page) + BLOCK_SIZE + sizeof(struct Node)) * BUFFER_SIZE);
	printf("\n---------------------------------------------------------------------------------------------------");

}


struct Page * find_page(int file_id, int block_id){
	for(int i; i < BUFFER_SIZE; i++){
		struct Page * page = &pages[i];
		if(page->file_id == file_id && page->block_id == block_id){
			return page;
		}
	}
	return NULL;
}


void request_page(int file_id, int block_id){
	struct Page * page = find_page(file_id, block_id);

	if(page == NULL){

	}else{

	}


}


void buffer_print_page(void* data){
	struct Page * page = (struct Page*) data;
	printf("\n-------------------------Frame ID: %d", page->frame_id);
	printf("\nCurrent Page ID: %d-%d",page->file_id, page->block_id);
	printf("\ndirty_flag: %c",page->dirty_flag);
	printf("\nstatus: %c",page->status);
	//printf("\ndata: %s", page->data);
	printf("\n-------------------------");
}


/* Examples using pointers
n == *p2 == **p1
&n == p2 == *p1
&p2 == p1
*/

#endif
