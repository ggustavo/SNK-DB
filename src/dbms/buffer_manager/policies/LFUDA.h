/*
 * Least Frequently Used with Dynamic Aging (LFU-DA)
 *
 * Reference Papers:
 *  - Intelligent Dynamic Aging Approaches in Web Proxy Cache Replacement (2015)
 *  - Evaluating Content Management Techniques for Web Proxy Caches (1999)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- LFUDA Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct LFUNode  |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * list;
unsigned long long int L = 0; // Inflation factor

struct LFUNode{
    struct Page * page;
    struct Node * node;
    unsigned long long int frequency;
    unsigned long long int k; // <--- K(P) = F(P) + L, where F is the frequency of page P 
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

	if(page != NULL){ //HIT - Update frequency and K

		struct LFUNode * lfu_node = (struct LFUNode *) page->extended_attributes;
        lfu_node->frequency =  lfu_node->frequency  + 1;
        lfu_node->k = lfu_node->frequency + L;

	} else { // MISS - page is not in Buffer (struct Page * page == NULL)

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct LFUNode * new_node = LFU_create_node(page);
            buffer_load_page(file_id, block_id, page); // Read the data from storage media
            LFU_insert(list, new_node);


		} else { // Need a replacement


            struct LFUNode * node_victim = get_victim(list);
            
            struct Page * victim = node_victim->page;

            printf("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->k);

			buffer_flush_page(victim); // Flush the data to the secondary storage media if is dirty
			page = buffer_reset_page(victim); // To avoid malloc a new page we reuse the victim page
			buffer_load_page(file_id, block_id, page); // Read new data from storage media

            node_victim->frequency = 1;
            node_victim->k = node_victim->frequency + L;
		}

	}
	set_dirty(page, operation);
	return page;
}

struct LFUNode * get_victim(struct List * list){
    
    struct Node * node = list->tail;
    struct LFUNode * victim = ((struct LFUNode *) node->content);
    
    int min = victim->k;
    int K = -1;

    while(node != NULL){
        K = ((struct LFUNode *) node->content)->k;
        
        if(K < min){
            min = K;
            victim = ((struct LFUNode *) node->content);
        }
        
        node = node->prev;
    }
    L = victim->k;
    return victim;
}

void LFU_insert(struct List * list, struct LFUNode * lfu_node){
	list_insert_node_head(list,lfu_node->node);
}

struct LFUNode * LFU_create_node(struct Page * page){
    struct LFUNode * lfu_node = (struct LFUNode *) malloc (sizeof(struct LFUNode));
    
    struct Node * new_node = list_create_node(lfu_node);
    
    lfu_node->frequency = 1;
    lfu_node->k = lfu_node->frequency + L;
    lfu_node->node = new_node;
    lfu_node->page = page;   

    page->extended_attributes = lfu_node;
    return lfu_node;
}


#endif
