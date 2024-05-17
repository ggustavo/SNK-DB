/*
 * Least Frequently Used (LFU)
 *
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- LFU Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct LFUNode  |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * list;


struct LFUNode{
    struct Page * page;
    struct Node * node;
    int reference;
};


struct LFUNode * LFU_create_node(struct Page * page);
void LFU_insert(struct List * list, struct LFUNode * node);
struct LFUNode * get_victim(struct List * list);

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

	if(page != NULL){ /* HIT - Update reference */

		struct LFUNode * lfu_node = (struct LFUNode *) page->extended_attributes;
        lfu_node->reference =  lfu_node->reference  + 1;

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct LFUNode * new_node = LFU_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            LFU_insert(list, new_node);


		} else { /*  Need a replacement */


            struct LFUNode * node_victim = get_victim(list);
            
            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->reference);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            node_victim->reference = 1;
		}

	}
	set_dirty(page, operation);
	return page;
}

struct LFUNode * get_victim(struct List * list){
    
    struct Node * node = list->tail;
    struct LFUNode * victim = ((struct LFUNode *) node->content);
    
    int min = victim->reference;
    int reference = -1;

    while(node != NULL){
        reference = ((struct LFUNode *) node->content)->reference;
        
        if(reference < min){
            min = reference;
            victim = ((struct LFUNode *) node->content);
        }
        
        node = node->prev;
    }

    return victim;
}

void LFU_insert(struct List * list, struct LFUNode * lfu_node){
	list_insert_node_head(list,lfu_node->node);
}

struct LFUNode * LFU_create_node(struct Page * page){
    struct LFUNode * lfu_node = (struct LFUNode *) malloc (sizeof(struct LFUNode));
    
    struct Node * new_node = list_create_node(lfu_node);
    
    lfu_node->reference = 1;
    lfu_node->node = new_node;
    lfu_node->page = page;   

    page->extended_attributes = lfu_node;
    return lfu_node;
}

void buffer_print_policy(){

}

#endif
