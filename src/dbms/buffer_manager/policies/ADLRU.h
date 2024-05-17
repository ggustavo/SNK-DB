/*
 * Adaptive Double LRU (AD-LRU)
 * 
 * Reference Paper:
 *  - AD-LRU: An efficient buffer replacement algorithm for flash-based databases (2012)
 */

#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- AD-LRU Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct WSRNode  |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * cold;
struct List * hot;
int min_lc;

struct WSRNode * FC_HOT = NULL;
struct WSRNode * FC_COLD = NULL;

struct WSRNode{
    struct Page * page;
    struct Node * node;
    int cold_bit;
};


struct WSRNode * WSR_create_node(struct Page * page);
void WSR_insert(struct List * list, struct WSRNode * node);
struct WSRNode * get_victim(struct WSRNode * FC, struct List * list);
struct WSRNode * ajust_FC(struct List* list);

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    min_lc = BUFFER_SIZE / 2; // TODO: 0.1 of BUFFER_SIZE 
    printf("\nBuffer Replacement Policy: %s\nmin_lc:%d", __FILE__, min_lc);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    cold = list_create(NULL,NULL);
    hot = list_create(NULL,NULL);
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------
    
	if(page != NULL){ /* HIT - Update reference */
        
        set_dirty(page, operation); // FOR FC WORK

		struct WSRNode * wsr_node = (struct WSRNode *) page->extended_attributes; 
        wsr_node->cold_bit = 0; //the cold bit is set to 0
        
        if(wsr_node->node->list == cold){
            
            list_remove(cold, wsr_node->node);
            WSR_insert(hot, wsr_node);

            ajust_FC(cold);
            ajust_FC(hot);


        }else

        if(wsr_node->node->list == hot){
            
            list_remove(hot, wsr_node->node);
            WSR_insert(hot, wsr_node);
            ajust_FC(hot);
            

        }else{
            printf("[ERR0] AD-LRU: The node is not on any list" );
            exit(1);
        }


	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct WSRNode * new_node = WSR_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            WSR_insert(cold, new_node);
            new_node->cold_bit = 0; //the cold bit is set to 0
           
            set_dirty(page, operation);
            ajust_FC(cold);

		} else { /*  Need a replacement */


            struct WSRNode * node_victim = NULL;
            
            if (cold->size > min_lc){
                node_victim = get_victim(FC_COLD, cold);
            }else{
                node_victim = get_victim(FC_HOT, hot);
            }

            if (node_victim == NULL){
                printf("\n[ERR0R] node_victim == NULL");
                exit(1);
            }

            if (node_victim->node->list != NULL){
                printf("\n[ERR0R] AD_LRU: The Node should have been removed from the list.");
                exit(1);
            }

            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->cold_bit);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            node_victim->cold_bit = 0;
            WSR_insert(cold, node_victim);
            
            set_dirty(page, operation);
            ajust_FC(cold);
		}

	}
    set_dirty(page, operation);
	return page;
}


struct WSRNode * get_victim(struct WSRNode * FC, struct List * list){
    
    if(FC != NULL){
        list_remove(list, FC->node);
		ajust_FC(list);
		return FC;
	}
    // select a dirty page using the socond-chance policy

    struct Node * node = list->tail;
    struct WSRNode * victim = ((struct WSRNode *) node->content);
    struct Page * page = victim->page;
    
     if(node == NULL){
		printf("\n[ERR0] AD-LRU: Victim Node is NULL");
		exit(1);
	}

    if(page->dirty_flag == PAGE_CLEAN){ //If the LRU page is clean
        printf("\n[ERR0] AD-LRU: FC null and the victim page is clean");
        exit(1);
    }

    if(victim->cold_bit == 1){ //If the LRU page is dirty with a cold bit equal to 1.
        list_remove(list, node);
        return victim; //The page is evicted

    }else{ // cold bit equal to 0
        list_remove(list, node); //second chance
        WSR_insert(list, victim); //page is moved to the MRU side
        victim->cold_bit = 1; // and the cold bit is set to 1
        return get_victim(FC, list); //recursive call
    }

    
    if(victim == NULL){
        printf("\n[ERR0] victim is NULL");
        exit(1);
    }

    return NULL;

}


struct WSRNode * ajust_FC(struct List* list){
    struct Node * node = list->tail;

	while (node != NULL) {
        struct WSRNode * wsr_node = (struct WSRNode *) node->content;
		if (wsr_node->page->dirty_flag == PAGE_CLEAN) {
			if (list == cold) {
				FC_COLD = wsr_node;
			} else if (list == hot) {
				FC_HOT = wsr_node;
			}else{
                printf("\n[ERR0] ajust_FC: list is not LC or LH");
                exit(1);
            }
			return wsr_node;
		}
		node = node->prev;
	}
	if(list == cold){
		FC_COLD = NULL;
	}else if(list == hot){
		FC_HOT = NULL;
	}else{
        printf("\n[ERR0] ajust_FC: list is not LC or LH");
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
    struct Node * x = cold->head;
    printf("\n<-");
    while(x!=NULL){
        struct WSRNode * wsr_node = (struct WSRNode *) x->content;
        struct Page * page = wsr_node->page;
        printf(" %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,wsr_node->cold_bit);
        x = x->next;
    }
    printf(" ->");
    x = hot->head;
    
    while(x!=NULL){
        struct WSRNode * wsr_node = (struct WSRNode *) x->content;
        struct Page * page = wsr_node->page;
        printf(" %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,wsr_node->cold_bit);
        x = x->next;
    }
    printf(" ->");
    if(FC_COLD != NULL){
        struct Page * page = FC_COLD->page;
        printf("FC_COLD: %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,FC_COLD->cold_bit);
    }
    if(FC_HOT != NULL){
        struct Page * page = FC_HOT->page;
        printf(" FC_HOT: %c[%d-%d]-%d", page->dirty_flag, page->file_id, page->block_id,FC_HOT->cold_bit);
    }
}

#endif
