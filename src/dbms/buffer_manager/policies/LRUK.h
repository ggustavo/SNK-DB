/*
 * Least kth-to-last Reference LRU-K
 * 
 * Reference Papers:
 *  - The lru-k page replacement algorithm for database disk buffering (1993)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

int K;
int GC;
int CRP;

struct List * list;

struct LRUKNode{
    struct Page * page;
    struct Node * node;
    int * tracks;
    int last;
};

struct LRUKNode * getVictim();
void insert_MRU(struct List * list, struct Node * node);
struct LRUKNode * create_lruk_node(struct Page * page);


/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    GC = 0;
    K = 2;
    CRP = 3;
    

    list = list_create(NULL,NULL);
    
    printf("\nBuffer Replacement Policy: %s \nK: %d CRP: %d", __FILE__,K,CRP);
    printf("\n---------------------------------------------------------------------------------------------------");

    
}

//
struct Page * buffer_request_page(int file_id, long block_id, char operation){
	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------
    GC ++;

	if(page != NULL){ /* HIT */
     
		struct LRUKNode * lruk_node = (struct LRUKNode *) page->extended_attributes;
     
        /* update history information of p */
        if (GC - lruk_node->last > CRP){ //if t – LAST(p) > Correlated Reference Period
            
            /* a new, uncorrelated reference */
            int CRP_page = lruk_node->last - lruk_node->tracks[0];  //correlation_period_of_referenced_page := LAST(p) – HIST(p,1)
           // printf("\nCRP_page: %d", CRP_page);
            for (int i = K-1; i > 0; i--){
                lruk_node->tracks[i] = lruk_node->tracks[i-1] + CRP_page; 
            }
            lruk_node->tracks[0] = GC; 
            lruk_node->last = GC;

        }else{
            //printf("\ncorrelated reference");
            /* a correlated reference */
            lruk_node->last = GC;
        }

        list_remove(list, lruk_node->node);
        insert_MRU(list, lruk_node->node);

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */
     
		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
            struct LRUKNode * lruk_node = create_lruk_node(page);
            insert_MRU(list, lruk_node->node);
            
            lruk_node->tracks[0] = GC;
            lruk_node->last = GC;

		} else { /* Need a replacement */

            
            struct LRUKNode * lruk_node = getVictim();
            list_remove(list, lruk_node->node);

			struct Page * victim = lruk_node->page; /* Get the LRU Page */
            printf("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);
			
            buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
			
            insert_MRU(list, lruk_node->node);
            // UPDATE HISTORY INFORMATION ?
            for (int i = K-1; i > 0; i--){
                lruk_node->tracks[i] = 0;
            }
            // ?
            lruk_node->tracks[0] = GC;
            lruk_node->last = GC;
		}

	}
	set_dirty(page, operation);
	return page;
}



/* select replacement victim */
struct LRUKNode * getVictim(){
    
    struct Node * node = list->tail;
    //struct LRUKNode * victim = ((struct LRUKNode *) node->content);
    struct LRUKNode * victim = NULL;
    struct LRUKNode * victim2 = NULL;

    /*
    int min = victim->tracks[K-1];
    int lowest = -1;

    while(node != NULL){
        lowest = ((struct LRUKNode *) node->content)->tracks[K-1];
        
        if(lowest < min){
            min = lowest;
            victim = ((struct LRUKNode *) node->content);
        }
        
        node = node->prev;
    }
    */

    int min = GC;
    int min2 = GC;

    struct LRUKNode * Q = NULL;

    while(node != NULL){ // for all pages q in the buffer do
        Q = (struct LRUKNode *) node->content;
        
        if (GC - Q->last > CRP /*eligible for replacement*/
          && Q->tracks[K-1] < min /* maximum Backward K–distance so far */
        ){ 
            victim = Q;
            min = Q->tracks[K-1];
        }else{ // in case all pages with CRP are not eligible

            if (Q->tracks[K-1] < min2){
                victim2 = Q;
                min2 = Q->tracks[K-1];
            }

        }
 
        node = node->prev;
    }

    if(victim == NULL){ // all pages with CRP are not eligible
        //printf("\n NO CRP eligible pages");
        victim = victim2;
    }

    return victim;
}

void insert_MRU(struct List * list, struct Node * node){
	list_insert_node_head(list,node);
}

struct LRUKNode * create_lruk_node(struct Page * page){

    struct LRUKNode * lruk_node = (struct LRUKNode *) malloc (sizeof(struct LRUKNode));
    
    struct Node * new_node = list_create_node(lruk_node);

    lruk_node->node = new_node;
    lruk_node->page = page;   
    page->extended_attributes = lruk_node;
    lruk_node->last = 0;
    lruk_node->tracks = (int *) malloc (sizeof(int) * K);
   
    for (int i = 0; i < K; i++){
       lruk_node->tracks[i] = 0; 
       //printf("\n ---- lruk_node->tracks[%d] = %d", i, lruk_node->tracks[i]);
    }
    
    return lruk_node;

}

void buffer_print_policy(){
    struct Node * x = list->head;
    printf("\n<-");
    while(x!=NULL){
        struct LRUKNode * n = (struct LRUKNode *) x->content;
        printf(" [%d-%ld]{", n->page->file_id, n->page->block_id);
        for (int i = 0; i < K; i++){
            printf("%d,", n->tracks[i]);            
        }
        printf("}");
        x = x->next;
    }
}

#endif
