/*
 * Cost-Aware Self-Adaptive (CASA)
 * 
 * Reference Paper:
 *  - Clean first or dirty first? A Cost-Aware Self-Adaptive buffer replacement policy (2010)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

void insert_MRU(struct List * list, struct Node * node);
struct Node * remove_LRU(struct List * list);
void move_to_MRU(struct List * source, struct Node * node, struct List * target);


struct List * C; // Clean List
struct List * D; // Dirty List
double advisor;
double CR; // Cost of Read
double CW; // Cost of Write

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    advisor = (double) BUFFER_SIZE / 2;
    CR = 0.5;
    CW = 0.5;

    printf("\nBuffer Replacement Policy: %s \nCR: %f CW: %f", __FILE__, CR, CW);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    C = list_create(NULL,NULL);
    D = list_create(NULL,NULL);
}


struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------
    

	if(page != NULL){ /* HIT - Update MRU */
        debug("   ADVISOR BEFORE: %f", advisor);
        
        struct Node * node = (struct Node *) page->extended_attributes;
        
		if( node->list == C && operation == READ_REQUEST){ // Hits in C with the read operation
            move_to_MRU(C, node, C); //The page is moved to the MRU of C

            //advisor τ is adjusted to τ = min(τ + CR × (|D| / |C|), B)
            advisor = (double) MIN(advisor + CR * SAFE_DIVISION( D->size, C->size), BUFFER_SIZE);
        }else 

        if( node->list == D && operation == WRITE_REQUEST){ // Hits in D with the write operation
            move_to_MRU(D, node, D); //The page is moved to the MRU of D

            //advisor τ is adjusted to τ = max(τ - CW × (|C| / |D|), 0)
            advisor = (double) MAX(advisor - CW * SAFE_DIVISION( C->size, D->size), 0);
        }else 

        if( node->list == C && operation == WRITE_REQUEST){ //Hits in C with the write operation
            move_to_MRU(C, node, D); //The page is moved to the MRU of D

        }else 

        if( node->list == D && operation == READ_REQUEST){ //Hits in D with the read operation
            move_to_MRU(D, node, D); //The page is moved to the MRU of D
            
        }else {
            printf("\n[ERR0] CASA: The page is not in the buffer?");
            exit(1);
        }
        
        debug(" AFTER: %f", advisor);

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct Node * new_node = list_create_node(page);
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
			
            if (operation == READ_REQUEST){
                insert_MRU(C, new_node);
            }else{
                insert_MRU(D, new_node);
            }
            page->extended_attributes = new_node;

		} else { /* Need a replacement */

			struct Node * lru_node = NULL;

            if( (double) C->size >= advisor){ // Using >= if the adivsor is equals the BUFFER_SIZE and D is empty
                lru_node = remove_LRU(C);
            }else{
                lru_node = remove_LRU(D);
            }

            if(lru_node == NULL){
                printf("\n[ERR0] CASA: lru_node is NULL C:%d D:%d A:%f", C->size, D->size,advisor);
                //buffer_print_policy();
                exit(1);
            }

			struct Page * victim = (struct Page *) lru_node->content; /* Get the LRU Page */
            
            if (victim == NULL){
                printf("\n[ERR0] CASA: victim is NULL");
                exit(1);
            }
            
            debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */

			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */

			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
			
        
            if (operation == READ_REQUEST){
                insert_MRU(C, lru_node);
            }else{
                insert_MRU(D, lru_node);
            }
            page->extended_attributes = lru_node;

		}

	}
	set_dirty(page, operation);
	return page;
}

void insert_MRU(struct List * list, struct Node * node){
	list_insert_node_head(list,node);
}

struct  Node * remove_LRU(struct List * list){
	return list_remove_tail(list);
}

void move_to_MRU(struct List * source,struct Node * node, struct List * target){
	list_remove(source, node);
	list_insert_node_head(target, node);
}

void buffer_print_policy(){
	struct Node * x = C->head;
    printf("\n<-");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" %c[%d-%d]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
    x = D->head;
    printf("->");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" %c[%d-%d]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
    //debug(" advisor: %f", advisor);
}

#endif
