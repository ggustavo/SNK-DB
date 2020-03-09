/*
 * Least Recently Used (LRU)
 *
 * Use this algorithm to develop new page replacement policies.
 *
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

void insert_MRU(struct List * list, struct Node * node);
struct Node * remove_LRU(struct List * list);
void move_to_MRU(struct List * list, struct Page * page);


struct List * list;

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    printf("\nBuffer Replacement Policy: %s", __FILE__);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    list = list_create(buffer_print_page,NULL);
}


struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ //HIT - Update MRU

		move_to_MRU(list, page);

	} else { // MISS - page is not in Buffer (struct Page * page == NULL)

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct Node * new_node = list_create_node(page);
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
			insert_MRU(list, new_node);

		} else { // Need a replacement

			printf("\n ---- REPLACEMENT ------ ");
			struct Node * lru_node = remove_LRU(list);
			struct Page * victim = (struct Page *) lru_node->content; //Get the LRU Page

			buffer_flush_page(victim); // Flush the data to the secondary storage media if is dirty

			page = buffer_reset_page(victim); // To avoid malloc a new page we reuse the victim page

			buffer_load_page(file_id, block_id, page); // Read new data from storage media
			insert_MRU(list, lru_node);

		}

	}
	set_dirty(page, operation);
	return page;
}

void insert_MRU(struct List * list, struct Node * node){
	list_insert_node_head(list,node);
	((struct Page*)node->content)->extended_attributes = node;
}

struct  Node * remove_LRU(struct List * list){
	return list_remove_tail(list);
}

void move_to_MRU(struct List * list, struct Page * page){
	struct Node * node = (struct Node *) page->extended_attributes;
	list_remove(list,node);
	list_insert_node_head(list,node);
}

#endif
