/*
 * Controllable Cold Clean First Least Recently Used (CCCF-LRU)
 * Reference Paper:
 * - The Implementation of Flash-aware Buffer Replacement Algorithms in PostgreSQL (2015)
 * 
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- CCFLRU Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |struct CCFLRUNode |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * mixed_list;
struct List * cold_clean;
int minCCL;

struct CCFLRUNode{
    struct Page * page;
    struct Node * node;
    int cold_bit;
};


struct CCFLRUNode * CCFLRU_create_node(struct Page * page);
void CCFLRU_insert(struct List * list, struct CCFLRUNode * node);
struct CCFLRUNode * get_dirty_victim();

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    minCCL = BUFFER_SIZE / 2;
    printf("\nBuffer Replacement Policy: %s\nminCCL: %d", __FILE__, minCCL);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    mixed_list = list_create(NULL,NULL);
    cold_clean = list_create(NULL,NULL);
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update reference */

		struct CCFLRUNode * ccflru_node = (struct CCFLRUNode *) page->extended_attributes;
        ccflru_node->cold_bit = 0; //the cold bit is set to 0
        // page is moved to MRU
        if(ccflru_node->node->list == cold_clean){
            list_remove(cold_clean, ccflru_node->node);
            CCFLRU_insert(mixed_list, ccflru_node);
        }else{
            list_remove(mixed_list, ccflru_node->node);
            CCFLRU_insert(mixed_list, ccflru_node);
        }


	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct CCFLRUNode * new_node = CCFLRU_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            if (operation == READ_REQUEST){
                new_node->cold_bit = 1;
                CCFLRU_insert(cold_clean, new_node);
            } else {
                new_node->cold_bit = 0;
                CCFLRU_insert(mixed_list, new_node);
            }

		} else { /*  Need a replacement */


            struct CCFLRUNode * node_victim = NULL;
           
            if(cold_clean->size < minCCL){ //If the size of cold clean is shorter than minCCL
                node_victim = get_dirty_victim(); //then selects a victim in mixed list.
                list_remove(mixed_list, node_victim->node);
            }else{
                node_victim = (struct  CCFLRUNode *) list_remove_tail(cold_clean)->content;
            }

            if (node_victim == NULL){
                printf("\nERROR: node_victim == NULL");
                exit(1);
            }
            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->cold_bit);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            if (operation == READ_REQUEST){
                node_victim->cold_bit = 1;
                CCFLRU_insert(cold_clean, node_victim);
            } else {
                node_victim->cold_bit = 0;
                CCFLRU_insert(mixed_list, node_victim);
            }
		}

	}
	set_dirty(page, operation);
	return page;
}

struct CCFLRUNode * get_dirty_victim(){
    
    struct Node * node = mixed_list->tail;
    struct CCFLRUNode * victim = ((struct CCFLRUNode *) node->content);
    struct Page * page = victim->page;
        
    if(victim->cold_bit == 1){ // If the LRU page has a cold bit equal to 1
            return victim; //evicted regardless of whether it is dirty or clean
    }else{ // cold bit equal to 0
        
        if(page->dirty_flag == PAGE_CLEAN){ // If the LRU page is clean
            list_remove(mixed_list, node); //second chance
            CCFLRU_insert( cold_clean, victim); //page is moved to the MRU side of the cold clean list
            victim->cold_bit = 1; // and the cold bit is set to 1
            return get_dirty_victim(); //recursive call
        }else{ // If the LRU page is dirty
            list_remove(mixed_list, node); //second chance
            CCFLRU_insert( mixed_list, victim); //page is moved to the MRU side of the mixed list
            victim->cold_bit = 1; // and the cold bit is set to 1
            return get_dirty_victim(); //recursive call
        }
            
            
    }

    if(victim == NULL){
        printf("\n[ERR0] victim is NULL");
        exit(1);
    }

    return NULL;
}

void CCFLRU_insert(struct List * list, struct CCFLRUNode * ccflru_node){
	list_insert_node_head(list,ccflru_node->node);
}

struct CCFLRUNode * CCFLRU_create_node(struct Page * page){
    struct CCFLRUNode * ccflru_node = (struct CCFLRUNode *) malloc (sizeof(struct CCFLRUNode));
    
    struct Node * new_node = list_create_node(ccflru_node);
    
    ccflru_node->cold_bit = -1;
    ccflru_node->node = new_node;
    ccflru_node->page = page;   

    page->extended_attributes = ccflru_node;
    return ccflru_node;
}


void buffer_print_policy(){
    struct Node * x = mixed_list->head;
    printf("\n<-");
    while(x!=NULL){
        struct CCFLRUNode * ccflru_node = (struct CCFLRUNode *) x->content;
        struct Page * page = ccflru_node->page;
        printf(" %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,ccflru_node->cold_bit);
        x = x->next;
    }
    printf(" ->");
    x = cold_clean->head;
    while(x!=NULL){
        struct CCFLRUNode * ccflru_node = (struct CCFLRUNode *) x->content;
        struct Page * page = ccflru_node->page;
        printf(" %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,ccflru_node->cold_bit);
        x = x->next;
    }
    printf(" ->");
}

#endif
