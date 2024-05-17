/*
 * LRU Write Sequence Reordering (LRU-WSR)
 * Reference Paper:
 * - LRU-WSR: integration of lru and writes sequence reordering for flash memory (2008)
 * 
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- LRU-WSR Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct WSRNode  |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * list;


struct WSRNode{
    struct Page * page;
    struct Node * node;
    int cold_bit;
};


struct WSRNode * WSR_create_node(struct Page * page);
void WSR_insert(struct List * list, struct WSRNode * node);
struct WSRNode * get_victim();


/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    printf("\nBuffer Replacement Policy: %s", __FILE__);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    list = list_create(NULL,NULL);
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update reference */

		struct WSRNode * wsr_node = (struct WSRNode *) page->extended_attributes;
        wsr_node->cold_bit = 0; //the cold bit is set to 0
        list_remove(list, wsr_node->node);
        WSR_insert(list, wsr_node); // page is moved to MRU

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct WSRNode * new_node = WSR_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            WSR_insert(list, new_node);


		} else { /*  Need a replacement */


            struct WSRNode * node_victim = get_victim();
            if (node_victim == NULL){
                printf("\nERROR: node_victim == NULL");
                exit(1);
            }
            list_remove(list, node_victim->node);
            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->cold_bit);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            node_victim->cold_bit = 0;
            WSR_insert(list, node_victim);
		}

	}
	set_dirty(page, operation);
	return page;
}

struct WSRNode * get_victim(){
    
    struct Node * node = list->tail;
    struct WSRNode * victim = ((struct WSRNode *) node->content);
    struct Page * page = victim->page;
        
    if(page->dirty_flag == PAGE_CLEAN){ //If the LRU page is clean
        return victim; //The page is evicted regardless of the cold bit value.
    
    }else{ 
        if(victim->cold_bit == 1){ //If the LRU page is dirty with a cold bit equal to 1.
            return victim; //The page is evicted
        }else{ // cold bit equal to 0
            list_remove(list, node); //second chance
            WSR_insert( list, victim); //page is moved to the MRU side
            victim->cold_bit = 1; // and the cold bit is set to 1
            return get_victim(); //recursive call
        }

    }

    if(victim == NULL){
        printf("\n[ERR0] victim is NULL");
        exit(1);
    }

    return NULL;
}

void WSR_insert(struct List * list, struct WSRNode * wsr_node){
	list_insert_node_head(list,wsr_node->node);
}

struct WSRNode * WSR_create_node(struct Page * page){
    struct WSRNode * wsr_node = (struct WSRNode *) malloc (sizeof(struct WSRNode));
    
    struct Node * new_node = list_create_node(wsr_node);
    
    wsr_node->cold_bit = 0;
    wsr_node->node = new_node;
    wsr_node->page = page;   

    page->extended_attributes = wsr_node;
    return wsr_node;
}


void buffer_print_policy(){
    struct Node * x = list->head;
    printf("\n<-");
    while(x!=NULL){
        struct WSRNode * wsr_node = (struct WSRNode *) x->content;
        struct Page * page = wsr_node->page;
        printf(" %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,wsr_node->cold_bit);
        x = x->next;
    }
    printf(" ->");
}

#endif
