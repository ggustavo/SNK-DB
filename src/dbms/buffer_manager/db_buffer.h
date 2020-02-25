/*
 * To reduce access to secondary storage media, the database uses a buffer manager
 * that makes copies of data in-memory and makes the query execution faster without
 * having to read or write in the secondary storage media every time.
 */

#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_config.h"
#include "../file_manager/db_file.h"
#include "../util/doubly_linked_list.h"

/*
 * STATUS_LOCKED -> When the query processor is performing an operation on the page (reading or writing),
 * otherwise: STATUS_UNLOCKED
 */
#define STATUS_LOCKED 'L'
#define STATUS_UNLOCKED 'U'

/*
 * DIRTY -> When the page received write operations (insert, update, delete),
 * otherwise: CLEAN
 */
#define DIRTY 'W'
#define CLEAN 'R'

/*
 * The Query Processor indicates that it needs to READ or WRITE when requesting a page.
 */
#define READ_REQUEST 'R'
#define WRITE_REQUEST 'W'

/*
 * Statistics used to evaluate the performance of the buffer manager
 */
unsigned long long int operations = 0; //Number of requests

unsigned long long int miss_operations = 0; //Number of requests when the data IS NOT in-memory
unsigned long long int hit_operations = 0;  //Number of requests when the data IS in-memory

unsigned long long int write_operations = 0; //Number of write requests
unsigned long long int read_operations = 0;  //Number of read requests

unsigned long long int flush_operations = 0; //Number of pages written on the secondary storage media
unsigned long long int load_operations = 0; //Number of pages loaded from secondary storage media to memory


struct Page{

	int file_id; //used to find the file on the secondary storage media
	long block_id; //used to find the block inside the file

	int frame_id; //frame id of page
	char * data; //pointer to allocated_memory frame

	char status; //status of page (STATUS_LOCKED or STATUS_UNLOCKED)
	char dirty_flag; //if the page is DIRTY or CLEAN

	void * extended_attributes; //used to add new attributes depending on the page replacement policy
};


/*
 * char * allocated_memory -> Pointer to the data allocated in the memory.
 * This array is divided into segments (frames) with a fixed size equal to BLOCK_SIZE
 *
 * struct Page * pages -> Pointer to an array with all pages
 */
char * allocated_memory;
struct Page * pages;
struct List * free_list;

/*
 * Reset the page to its original state. Note: This does not reset the data allocated in-memory for the frame.
 */
struct Page * buffer_reset_page(struct Page *page) {
	if (page != NULL) {
		page->block_id = -1;
		page->file_id = -1;
		page->dirty_flag = CLEAN;
		page->status = STATUS_UNLOCKED;
		return page;
	}
	printf("\n[ERR0] CLean Page NULL");
	return NULL;
}

/*
 * Initializes in-memory storage structures of the buffer manager.
 */
void buffer_start() {
	free_list = list_create(NULL,NULL);
	allocated_memory = (char*) malloc( BUFFER_SIZE * BLOCK_SIZE );
	pages = (struct Page*) malloc(sizeof(struct Page) * BUFFER_SIZE);

	//BUFFER_SIZE * BLOCK_SIZE determines the size of the space allocated in-memory for data
	int N = BUFFER_SIZE * BLOCK_SIZE;

	for (int i = 0; i < N; i++) { //for each allocated byte
		allocated_memory[i] = 0; //initialize the byte with 0

		if (i % BLOCK_SIZE == 0) {
			//example: if the block_size is equal to 10, this "if" is true when i = {0, 10, 20, ...}

			//The idea is to map each frame to be managed by a page
			struct Page * page = &pages[i / BLOCK_SIZE];
			page->frame_id = i / BLOCK_SIZE;
			page->data = &allocated_memory[i]; //set the first pointer of this frame into the page
			page->extended_attributes = NULL;
			buffer_reset_page(page);
			list_insert_tail(free_list,page);
		}

	}
	printf("\n---------------------------------------------------------------------------------------------------");
	//printf("\nBuffer - Size of C Pointers: %d bytes", sizeof(void*));
	printf("\nBuffer - Number of Pages: %d", BUFFER_SIZE);
	printf("\nBuffer - Size of Block: %d bytes", BLOCK_SIZE);
	printf("\nBuffer - Size of Page (without data): %d bytes",sizeof(struct Page));
	printf("\nBuffer - Data Allocated Memory ( Size of Block * Number of Pages ): %d bytes",(BUFFER_SIZE * BLOCK_SIZE));
	printf("\nBuffer - Total ( [Size of Page + Size of Block] * Number of Pages ): %d bytes",(sizeof(struct Page) + BLOCK_SIZE) * BUFFER_SIZE);
	printf("\n---------------------------------------------------------------------------------------------------");

}

/*
 * This function needs to be optimized for a hash table or something faster than a sequential search.
 */
struct Page * buffer_find_page(int file_id, long block_id){
	for(int i = 0; i < BUFFER_SIZE; i++){
		struct Page * page = &pages[i];
		if(page->file_id == file_id && page->block_id == block_id){
			return page;
		}
	}
	return NULL;
}

/*
 * If page is DIRTY this function writes the data on the secondary storage media.
 */
struct Page * buffer_flush_page(struct Page * target){
	if(target != NULL){
		if(target->dirty_flag == DIRTY){
			flush_operations++;
			file_write(target->file_id, target->block_id, target->data, BLOCK_SIZE);
		}
		return target;
	}
	printf("\n[ERR0] Flush Page NULL");
	return NULL;
}

/*
 * Loads the page from secondary storage media to memory
 */
struct Page * buffer_load_page(int file_id, long block_id, struct Page * target) {
	if (target != NULL) {
		buffer_reset_page(target);
		load_operations++;
		file_read(file_id, block_id, target->data, BLOCK_SIZE);
		target->file_id = file_id;
		target->block_id = block_id;
		return target;
	}
	printf("\n[ERR0] Load Page %d#%ld NULL",file_id,block_id);
	return NULL;
}



/*
 * computes the statistics of requests (operations, miss, hits, reads, writes)
 * Rule -> operations = (miss + hits) = (reads + writes)
 */
void buffer_computes_request_statistics(struct Page * page, char operation){

	operations++;

	if(page == NULL){
		miss_operations++;
	}else{
		hit_operations++;
	}
	if(operation == READ_REQUEST){
		read_operations++;
	}else{
		write_operations++;
	}

}


char buffer_is_full(){
	if(free_list->size < 0){
		printf("\n[ERR0] Free List negative size ...");
	}
	if(free_list->size == 0){
		return TRUE;
	}
	return FALSE;
}


struct Page * buffer_get_free_page(){
	if(buffer_is_full() == TRUE){
		printf("\n[ERR0] Get Free Pages ...");
		return NULL;
	}
	struct Node * node = list_remove_head(free_list);
	struct Page * page = (struct Page *) node->content;
	list_free_node(free_list, node);
	return page;
}

void buffer_add_new_free_page(struct Page * page){
	buffer_reset_page(page);
	list_insert_tail(free_list,page);
}


void set_dirty(struct Page * page, char operation){
	if(operation == DIRTY && page->dirty_flag == CLEAN){
		page->dirty_flag = DIRTY;
	}
}


void buffer_print_page(void* data){
	struct Page * page = (struct Page*) data;
	if(page->file_id == -1){
		printf("(NULL), ");
	}else{
		printf("%c(%d#%ld), ",page->dirty_flag,page->file_id, page->block_id);
	}
}

void buffer_print_page_complete(void* data){
	struct Page * page = (struct Page*) data;
	printf("\n-------------------------Frame ID: %d", page->frame_id);
	printf("\nCurrent Page ID: %d#%ld",page->file_id, page->block_id);
	printf("\ndirty_flag: %c",page->dirty_flag);
	printf("\nstatus: %c",page->status);
	printf("\ndata: ");
	for(int i = 0; i < BLOCK_SIZE; i++){
		printf("%c",page->data[i]);
	}

	printf("\n-------------------------");
}


void buffer_print_statistics(){ //%llu for Linux, %I64u for Windows
	printf("\n-------------------Buffer Statistics---------------------------");
	printf("\nOperations: %I64u ",operations);
	printf("(hit: %I64u miss: %I64u) ", hit_operations, miss_operations);
	printf("(write: %I64u read: %I64u)", write_operations, read_operations);
	printf("\nloaded: %I64u flushed: %I64u", load_operations, flush_operations);
	printf("\n---------------------------------------------------------------");
}

/* Examples using pointers
	n == *p2 == **p1
	&n == p2 == *p1
	&p2 == p1
*/

#endif