/*
 * Locality-aware Least Recently Used (LLRU)
 * Reference Paper:
 * - Locality-Aware Replacement Algorithm in Flash Memory to Optimize Cloud Computing for Smart Factory of Industry 4.0 (2017)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- LLRU Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct LLRUNode  |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * hot_clean;
struct List * hot_dirty;
struct List * cold_clean;
struct List * cold_dirty;

double ECC; // evict cost for clean page
double ECD; // evict cost for dirty page


struct LLRUNode{
    struct Page * page;
    struct Node * node;
    int references;
};


struct LLRUNode * LLRU_create_node(struct Page * page);
void LLRU_insert(struct List * list, struct LLRUNode * node);
struct LLRUNode * get_LLRUNode(struct Node * node);
double LLRU_PC(struct LLRUNode * lru_node);
struct LLRUNode * get_victim();
void LLRU_print_PC(struct Node * x);

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
	ECC = 0.3; // ??
    ECD = 0.7; // ??
    printf("\nBuffer Replacement Policy: %s\nECC:%f ECD:%f", __FILE__, ECC, ECD);
    printf("\n---------------------------------------------------------------------------------------------------");
    hot_clean  = list_create(NULL,NULL);
    hot_dirty  = list_create(NULL,NULL);
    cold_clean = list_create(NULL,NULL);
    cold_dirty = list_create(NULL,NULL);
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update references */

		struct LLRUNode * llru_node = (struct LLRUNode *) page->extended_attributes;
        llru_node->references =  llru_node->references  + 1;

        list_remove(llru_node->node->list,llru_node->node); // Removes the node from the current list

        if (operation == READ_REQUEST && page->dirty_flag == PAGE_CLEAN){
            LLRU_insert(hot_clean, llru_node);
        }else{
            LLRU_insert(hot_dirty, llru_node);
        }
        
	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct LLRUNode * new_node = LLRU_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            
            if (operation == READ_REQUEST){
                LLRU_insert(cold_clean, new_node);
            }else{
                LLRU_insert(cold_dirty, new_node);
            }

		} else { /*  Need a replacement */


            struct LLRUNode * node_victim = get_victim();
            list_remove(node_victim->node->list, node_victim->node);

            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->references);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            node_victim->references = 1;

            if (operation == READ_REQUEST){
                LLRU_insert(cold_clean, node_victim);
            }else{
                LLRU_insert(cold_dirty, node_victim);
            }
		}

	}
	set_dirty(page, operation);
	return page;
}

struct LLRUNode * get_LLRUNode(struct Node * node){
    if (node == NULL) return NULL;
    return (struct LLRUNode *) node->content;
}

struct LLRUNode * get_victim(){
    
    struct LLRUNode * P_CC = get_LLRUNode(cold_clean->tail);
    struct LLRUNode * P_CD = get_LLRUNode(cold_dirty->tail);
    struct LLRUNode * P_HC = get_LLRUNode(hot_clean->tail);
    struct LLRUNode * P_HD = get_LLRUNode(hot_dirty->tail);

    struct LLRUNode * nodes[4] = {P_CC, P_CD, P_HC, P_HD};
   
    struct LLRUNode * node_victim = NULL;
    double min_pc = -1;

    for(int i = 0; i < 4; i++){
        if(nodes[i] != NULL){
            double pc = LLRU_PC(nodes[i]);
            if(pc < min_pc || min_pc == -1){
                min_pc = pc;
                node_victim = nodes[i];
            }
        }
    }

    if (node_victim == NULL){
        printf("\n[ERR0] LLRU: No victim found");
        exit(1);
    }

    return node_victim;
}

double LLRU_PC(struct LLRUNode * lru_node){
    if (lru_node == NULL){
        printf("\n[ERR0] LLRU: PC calc -> node is NULL");
        exit(1);
    }

    if(lru_node->node->list == NULL){
        printf("\n[ERR0] LLRU: PC calc -> node->list is NULL");
        exit(1);
    }
    double COST = 0;
    if (lru_node->node->list == cold_clean || lru_node->node->list == hot_clean){
        COST = ECC; 
    }else
    
    if (lru_node->node->list == cold_dirty || lru_node->node->list == hot_dirty){
        COST = ECD; 
    }else{
        printf("\n[ERR0] LLRU: PC calc Node not found in any list");
    }
    return (double) lru_node->references * COST;
}

void LLRU_insert(struct List * list, struct LLRUNode * llru_node){
    list_insert_node_head(list,llru_node->node);
}

struct LLRUNode * LLRU_create_node(struct Page * page){
    struct LLRUNode * llru_node = (struct LLRUNode *) malloc (sizeof(struct LLRUNode));
    
    struct Node * new_node = list_create_node(llru_node);
    
    llru_node->references = 1;
    llru_node->node = new_node;
    llru_node->page = page;   

    page->extended_attributes = llru_node;
    return llru_node;
}

void buffer_print_policy(){
    struct Node * x = hot_clean->tail;
    printf("\n");
    while(x!=NULL){
        struct LLRUNode * n = (struct LLRUNode *) x->content;
        printf(" %c[%d-%ld]:%d",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->prev;
    }
    x = cold_clean->tail;
    printf(" <-");
    while(x!=NULL){
        struct LLRUNode * n = (struct LLRUNode *) x->content;
        printf(" %c[%d-%ld]:%d",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->prev;
    }
    x = cold_dirty->head;
    printf(" <- ... ->");
    while(x!=NULL){
        struct LLRUNode * n = (struct LLRUNode *) x->content;
        printf(" %c[%d-%ld]:%d",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->next;
    }
    x = hot_dirty->head;
    printf(" ->");
    while(x!=NULL){
        struct LLRUNode * n = (struct LLRUNode *) x->content;
        printf(" %c[%d-%ld]:%d",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->next;
    }
    printf("  {");
    LLRU_print_PC(hot_clean->tail); printf(",");
    LLRU_print_PC(cold_clean->tail);printf(",");
    LLRU_print_PC(cold_dirty->tail);printf(",");
    LLRU_print_PC(hot_dirty->tail);
    printf(" }");
}

void LLRU_print_PC(struct Node * x){
    if(x == NULL){
        printf(" NULL");
        return;
    }
    struct LLRUNode * n = (struct LLRUNode *) x->content;
    double PC = LLRU_PC(n);
    printf(" %c[%d-%ld]:%d = %f",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references, PC);
}

#endif
