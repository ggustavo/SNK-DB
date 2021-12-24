/*
 *  Ghost buffer Assisted and Self-tuning Algorithm (GASA)
 * 
 * Reference Paper:
 * - GASA: A New Page Replacement Algorithm for NAND Flash Memory (2016)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#define LIR 0
#define HIR 1

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"


struct List * cold_clean_list;
struct List * mixed_list;
struct List * ghost_list;
int GSMIN;
int GSMAX;
int GS; //Ghost List adivisor

struct GASANode{
    struct Page * page;
    struct Node * node;
    int hot_flag;
    int ghost_bit;

    int g_file_id;
    int g_block_id;
    char g_dirty_flag;
};


struct GASANode * GASA_create_node(struct Page * page);
struct GASANode * get_victim();
struct GASANode * find_ghost(int file_id, int block_id);
void GASA_insert(struct List * list, struct GASANode * gasa_node);
void GASA_ajust_GL();

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    //10% and 100%
    GSMIN = MAX( 1, BUFFER_SIZE * 0.1 );
    GSMAX = BUFFER_SIZE;
    GS = GSMAX / 2;
    printf("\nBuffer Replacement Policy: %s \nGSMIN: %d GSMAX: %d", __FILE__, GSMIN, GSMAX);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    cold_clean_list = list_create(NULL,NULL);
    mixed_list = list_create(NULL,NULL);
    ghost_list = list_create(NULL,NULL);
 
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------
    

	if(page != NULL){ /* HIT - Update reference */

		struct GASANode * gasa_node = (struct GASANode *) page->extended_attributes;
        
        if(gasa_node->node->list == ghost_list){
            printf("\n[ERR0R] Ghost page found in Real Buffer");
            exit(1);
        }

        list_remove(gasa_node->node->list, gasa_node->node);
	    list_insert_node_head(mixed_list, gasa_node->node);         
        gasa_node->hot_flag = 1;

        if(gasa_node->ghost_bit == 1){
            GS = MIN(GS + 1, GSMAX);
            gasa_node->ghost_bit = 0;
        }

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {
        
			page = buffer_get_free_page();
			struct GASANode * new_node = GASA_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            
            if (operation == READ_REQUEST){
                GASA_insert(cold_clean_list, new_node);
            }else{
                GASA_insert(mixed_list, new_node);
            }
        
		} else { /*  Need a replacement */
           
           
            struct GASANode * ghost = find_ghost(file_id, block_id);
            struct GASANode * node_victim = get_victim();
         
            list_remove(node_victim->node->list, node_victim->node);
            //if(operations == 11){printf("\nFINISH");exit(1);}
            
            struct Page * victim = node_victim->page;
            if(node_victim->ghost_bit == 1){
                GS = MAX(GSMIN , GS - (int) SAFE_DIVISION(GSMAX, (GSMAX-GS+1) ) );
            }
 
            debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

            //Hits in the ghost list, the page is loaded
            //into memory and moved to the MRU of the mixed list, 
            //and the ghost-bit and hot-flag are set to 1.
            int ghost_hit_flag = FALSE;
            if(ghost != NULL){
                ghost_hit_flag = TRUE; //
                node_victim->ghost_bit = 1;
                node_victim->hot_flag = 1;
                list_remove(ghost->node->list, ghost->node);

            }else{
                node_victim->ghost_bit = 0;
                node_victim->hot_flag = 0;
                ghost = GASA_create_node(NULL);
            }

            ghost->g_block_id = victim->block_id;
            ghost->g_file_id = victim->file_id; 
            ghost->g_dirty_flag = victim->dirty_flag;
            ghost->ghost_bit = 1;
            list_insert_node_head(ghost_list, ghost->node);

            buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

           
            if (ghost_hit_flag == FALSE
                && operation == READ_REQUEST 
                && victim->dirty_flag == PAGE_CLEAN ){

                GASA_insert(cold_clean_list, node_victim);
            }else{
    
                GASA_insert(mixed_list, node_victim);
            }
            
            GASA_ajust_GL();
		}

	}
	set_dirty(page, operation);
	return page;
}

void GASA_ajust_GL(){
    if(ghost_list->size > GS){
        struct Node * node = list_remove_tail(ghost_list);
        struct GASANode * ghost_node = (struct GASANode *) node->content;
        if(ghost_node->page != NULL){
            printf("\n[ERR0] Page found in Ghost Buffer");
            exit(1);
        }
        list_free_node(ghost_list, node);
        ghost_node->node = NULL;
        ghost_node->page = NULL;
        free(ghost_node);
        GASA_ajust_GL();
    }
}

struct GASANode * get_victim(){
    
    struct GASANode * victim = NULL;

    if(cold_clean_list->size > 0){ // if the cold clean list is not empty
        victim = (struct GASANode *) cold_clean_list->tail->content; //cold-clean LRU as a victim
    }else{
        struct Node * node = mixed_list->tail;
        victim = (struct GASANode *) node->content;
        //selects the LRU from the mixed list as a victim if the hot-flag equals 0
        if (victim->hot_flag == 0){
            return victim;
        }else{ // If the hot-flag equals 1
            list_remove(mixed_list, node); //second chance
            victim->hot_flag = 0; // setting the hot-flag to 0
            //moves the page to a cold clean list or mixed list depending on the type of operation
            if(victim->page->dirty_flag == PAGE_CLEAN){
                GASA_insert(cold_clean_list, victim);
            }else{
                GASA_insert(mixed_list, victim);
            }
            return get_victim();
        }
    }

    if (victim == NULL){
        printf("\n[ERR0] victim == NULL");
        exit(1);
    }

    return victim;
}

struct GASANode * find_ghost(int file_id, int block_id){
    struct Node * x = ghost_list->head;
        while(x != NULL){
            struct GASANode * ghost = (struct GASANode *) x->content;
            if (ghost->page != NULL){
                printf("\n[ERR0] ghost->page != NULL");
                exit(1);
            }
           
            if(ghost->g_file_id == file_id && ghost->g_block_id == block_id){
                return ghost;
            }
            x = x->next;
        }
    return NULL;
}



struct GASANode * GASA_create_node(struct Page * page){
    struct GASANode * gasa_node = (struct GASANode *) malloc (sizeof(struct GASANode));
    struct Node * new_node = list_create_node(gasa_node);

    gasa_node->node = new_node;
    gasa_node->page = page;

    gasa_node->ghost_bit = 0;
    gasa_node->hot_flag = 0;

    gasa_node->g_file_id = -1;
    gasa_node->g_block_id = -1;
    gasa_node->g_dirty_flag = -1;
    if(page != NULL){
        page->extended_attributes = gasa_node;
    }
    return gasa_node;
}

void GASA_insert(struct List * list, struct GASANode * gasa_node){
    list_insert_node_head(list ,gasa_node->node);
}

void buffer_print_policy(){
  
    struct Node * x = cold_clean_list->head;
    printf("\nC->");
    while(x!=NULL){
        struct GASANode * gasa_node = (struct GASANode *) x->content;
        printf(" %c[%d-%ld]%d,%d", gasa_node->page->dirty_flag, gasa_node->page->file_id, gasa_node->page->block_id,gasa_node->hot_flag,gasa_node->ghost_bit);
        x = x->next;
    }

    printf(" M->");
    x = mixed_list->head;
    while(x!=NULL){
        struct GASANode * gasa_node = (struct GASANode *) x->content;
        printf(" %c[%d-%ld]%d,%d", gasa_node->page->dirty_flag, gasa_node->page->file_id, gasa_node->page->block_id,gasa_node->hot_flag,gasa_node->ghost_bit);
        x = x->next;
    }
    printf(" G->");
    x = ghost_list->head;
    while(x!=NULL){
        struct GASANode * gasa_node = (struct GASANode *) x->content;
        printf(" %c[%d-%ld]%d,%d", gasa_node->g_dirty_flag, gasa_node->g_file_id, gasa_node->g_block_id,gasa_node->hot_flag,gasa_node->ghost_bit);
        x = x->next;
    }
    printf(" ->  GSMIN:%d, GS:%d, GSMAX:%d", GSMIN, GS, GSMAX);

    
}

#endif
