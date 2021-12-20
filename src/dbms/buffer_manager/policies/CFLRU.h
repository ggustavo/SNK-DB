/*
 * Clean-First LRU (CFLRU)
 *
 * Reference Paper:
 * CFLRU: A replacement algorithm for flash memory (2006)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

void insert_MRU(struct List * list, struct Node * node);
struct Node * remove_LRU(struct List * list);
struct Node * get_victim();


//REGIONS
struct List * working;
struct List * clean_first;
int W;

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    W = BUFFER_SIZE / 2;
    printf("\nBuffer Replacement Policy: %s \nW:%d", __FILE__, W);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    working = list_create(NULL,NULL);
    clean_first = list_create(NULL,NULL);
}


struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update MRU */

        struct Node * node = (struct Node *) page->extended_attributes;
	    list_remove(node->list, node);
	    insert_MRU(working, node); // move to WORKING REGION MRU
		
        if(working->size >= BUFFER_SIZE - W){
            struct Node * lru_node = remove_LRU(working);
            insert_MRU(clean_first, lru_node);
        }

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct Node * new_node = list_create_node(page);
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
			
            if(working->size >= BUFFER_SIZE - W){
                struct Node * lru_node = remove_LRU(working);
                insert_MRU(clean_first, lru_node);
            }
            insert_MRU(working, new_node);

		} else { /* Need a replacement */

			struct Node * victim_node = get_victim();
			

            struct Page * victim = (struct Page *) victim_node->content; /* Get the LRU Page */
			printf("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
			
            if(working->size >= BUFFER_SIZE - W){
                struct Node * lru_node = remove_LRU(working);
                insert_MRU(clean_first, lru_node);
            }

            insert_MRU(working, victim_node);

		}

	}
	set_dirty(page, operation);
	return page;
}

struct Node * get_victim(){
    struct Node * node = clean_first->tail;
    
    
    while(node != NULL){

        if ( ( (struct Page *) node->content)->dirty_flag == PAGE_CLEAN ){
            list_remove(clean_first, node);
            return node;
        }
        node = node->prev;
    }
    
    
    if (node == NULL){
        node = remove_LRU(clean_first);
    }

    if (node == NULL){
        printf("\n[ERR0] get_victim() NULL");
        exit(1);
    }
    return node;
}

void insert_MRU(struct List * list, struct Node * node){
	list_insert_node_head(list,node);
	((struct Page*)node->content)->extended_attributes = node;
}

struct  Node * remove_LRU(struct List * list){
	return list_remove_tail(list);
}

void print_CFLRU(){
    struct Node * x = working->head;
    printf("\n->");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" %c[%d-%ld]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
    x = clean_first->head;
    printf(" ->");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" %c[%d-%ld]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
}

#endif
