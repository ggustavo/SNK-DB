/*
 * Generalized Clock (GCLOCK)
 * 
 * Reference Paper:
 * - Sequentiality and prefetching in Database Systems (1978)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- Clock Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               | struct ClockNode |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * list;

int IC; // INITIAL_COUNT
int HC; // HIT_COUNT

struct ClockNode{
    struct Page * page;
    struct Node * node;
    int reference;
};



struct ClockNode * clock_next();
struct ClockNode * clock_create_node(struct Page * page);
void clock_insert(struct ClockNode * clock_node);


struct ClockNode * clock_pointer;

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    printf("\nBuffer Replacement Policy: %s", __FILE__);
    printf("\n---------------------------------------------------------------------------------------------------");
    IC = 1;
    HC = 1;

    list = list_create(buffer_print_page,NULL);
    clock_pointer = NULL;
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update reference */

		struct ClockNode * clock_node = (struct ClockNode *) page->extended_attributes;
        clock_node->reference =  HC;

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct ClockNode * new_node = clock_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            clock_insert(new_node);

		} else { /* Need a replacement */

            while(clock_pointer->reference != 0){ /* finds an element with a reference == 0 */
                clock_pointer->reference = clock_pointer->reference - 1;
                clock_pointer = clock_next();
            }

            struct Page * victim = clock_pointer->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag,victim->file_id,victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /*  To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */
			
            clock_pointer->reference = IC;
            clock_pointer = clock_next();

		}

	}
	set_dirty(page, operation);
	return page;
}

struct ClockNode * clock_next(){
    struct Node * node = clock_pointer->node->next;

    if(node == NULL){ 
        node = list->head; 
    }
    return (struct ClockNode *) node->content;
}

struct ClockNode * clock_create_node(struct Page * page){
    struct ClockNode * clock_node = (struct ClockNode *) malloc (sizeof(struct ClockNode));
    
    struct Node * new_node = list_create_node(clock_node);
    
    clock_node->reference = IC;
    clock_node->node = new_node;
    clock_node->page = page;   

    page->extended_attributes = clock_node;
    return clock_node;
}

void clock_insert(struct ClockNode * clock_node){

    list_insert_node_tail(list, clock_node->node);
    clock_pointer = clock_node;
}

void buffer_print_policy(){
    
}


#endif
