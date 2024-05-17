/*
 * LRU with Midpoint Insertion Strategy (LRU-MIS)
 *
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"


struct Node * remove_LRU(struct List * list);
void insert_Hot(struct Node * node);
void insert_Cold(struct Node * node);

struct List * hot;
struct List * cold;
int hot_size;
int cold_size;

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){

    hot_size = (BUFFER_SIZE * 5) / 8; //62.5%
    cold_size = BUFFER_SIZE - hot_size;

    printf("\nBuffer Replacement Policy: %s \ncold: %d hot: %d", __FILE__, cold_size, hot_size);
    printf("\n---------------------------------------------------------------------------------------------------");
    
    hot = list_create(NULL,NULL);
    cold = list_create(NULL,NULL);

}


struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update MRU */
        struct Node * node = (struct Node *) page->extended_attributes;
		list_remove(node->list,node);
	    insert_Hot(node);

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct Node * new_node = list_create_node(page);
            page->extended_attributes = new_node;
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
			insert_Cold(new_node);

		} else { /* Need a replacement */

			struct Node * lru_node = remove_LRU(cold);
			struct Page * victim = (struct Page *) lru_node->content; /* Get the LRU Page */
			
            if(lru_node == NULL){
                printf("\n[ERR0] LRU_MIS: lru_node == NULL");
                exit(1);
            }
            
            debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);


			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */

			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */

			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
			
            insert_Cold(lru_node);

		}

	}
	set_dirty(page, operation);
	return page;
}

void insert_Hot(struct Node * node){

    if(hot->size >= hot_size){
        struct Node * victim = list_remove_tail(hot);
        insert_Cold(victim);
    }
    
    list_insert_node_head(hot,node);
}

void insert_Cold(struct Node * node){
	
    if(hot->size > hot_size){
       printf("\n[ERR0] Inserting in the hot list with maximum size already reached");
       exit(1);
    }
    
    list_insert_node_head(cold,node);
}


struct  Node * remove_LRU(struct List * list){
	return list_remove_tail(list);
}


void buffer_print_policy(){
    struct Node * x = hot->head;
    printf("\n<-");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" [%d-%ld]", page->file_id, page->block_id);
        x = x->next;
    }
    printf(" ->");
    x = cold->head;
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" [%d-%ld]", page->file_id, page->block_id);
        x = x->next;
    }
    

}

#endif
