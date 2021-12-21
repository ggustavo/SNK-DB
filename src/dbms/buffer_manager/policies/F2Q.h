/*
 * 2Q, Full Version
 * 
 * Reference Paper: 
 * - A low overhead high performance buffer management replacement algoritm
 */

#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"


struct List * A1in;
struct List * Am;
struct List * A1out;

int Kout;
int Kin;

struct List * free_ghost_nodes;

struct A1OutGhostNode{
    struct Node * node;
    int file_id;
    long block_id;
};


struct A1OutGhostNode * get_ghost_page();
void insert_ghost_page(struct A1OutGhostNode * ghost );
void insert_MRU(struct List * list, struct Node  * node);
struct Node  * remove_LRU(struct List * list);
struct A1OutGhostNode * find_ghost_node(struct List * list, int file_id, int block_id);


/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){

    Kin = BUFFER_SIZE / 4; // 25%
    Kout = BUFFER_SIZE / 2; // 50%
   
    printf("\nBuffer Replacement Policy: %s \nkin: %d kout: %d", __FILE__, Kin,Kout);
    printf("\n---------------------------------------------------------------------------------------------------");
    
    A1in = list_create(NULL,NULL);
    Am = list_create(NULL,NULL);
    A1out = list_create(NULL,NULL);

    free_ghost_nodes = list_create(NULL,NULL);

    for (int i = 0; i < Kout; i++){
        struct A1OutGhostNode * ghost = (struct A1OutGhostNode *) malloc (sizeof( struct A1OutGhostNode));
        ghost->file_id = -2;
        ghost->block_id = -2;
        ghost->node = list_create_node(ghost);
        list_insert_node_head(free_ghost_nodes, ghost->node);
    }
    
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT */

		struct Node * node = (struct Node *) page->extended_attributes;
        
        if (node->list == Am){ // move to Am MRU
           list_remove(node->list, node);
           insert_MRU(Am, node);
        } //else do nothing

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */


        struct A1OutGhostNode * ghost = find_ghost_node(A1out, file_id, block_id);
        
        if(ghost != NULL){ /* GHOST HIT !! */
            
            struct Page * victim = NULL;
            struct Node * node_victim = NULL;
            struct List * victim_list = NULL;

            if(A1in->size > Kin){
                node_victim = remove_LRU(A1in);
                victim_list = A1in;
                victim = (struct Page*) node_victim->content;
            }else{
                node_victim = remove_LRU(Am);    
                victim_list = Am;    
                victim = (struct Page*) node_victim->content;
            }

            printf("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

            if (victim_list == A1in){
                list_remove(A1out, ghost->node);    /* Removes hitted ghost */
                /* We reuse the hitted ghost structure and update the victim's information on it */
                ghost->file_id  = victim->file_id;  /* Update file_id */
                ghost->block_id = victim->block_id; /* Update block_id */
                insert_MRU(A1out, ghost->node);     /* Insert "new Ghost" */ 
            }else{
                list_remove(A1out, ghost->node);
                ghost->file_id  = -1;  /* Update file_id */
                ghost->block_id = -1;  /* Update block_id */
                insert_MRU(free_ghost_nodes, ghost->node);
            }

            buffer_flush_page(victim);
            page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
            buffer_load_page(file_id, block_id, page);   /* Read the data from storage media */
            insert_MRU(Am, (struct Node*) page->extended_attributes); 

        }else


		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
            struct Node * new_node = list_create_node(page);
            page->extended_attributes = new_node;
            buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            insert_MRU(A1in, new_node);


		} else { /* Need a replacement */

            struct Page * victim = NULL;
            struct Node * node_victim = NULL;
            
            if(A1in->size > Kin){
                node_victim = remove_LRU(A1in);
                victim = (struct Page*) node_victim->content;

                ghost = get_ghost_page();
                ghost->file_id  = victim->file_id;  /* Update file_id */
                ghost->block_id = victim->block_id; /* Update block_id */
                insert_ghost_page(ghost);           /* Insert Ghost*/

            }else{
                node_victim = remove_LRU(Am);    
                victim = (struct Page*) node_victim->content;    
            }
            
            
            printf("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);
            
            buffer_flush_page(victim);
            page = buffer_reset_page(victim);
			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
            insert_MRU(A1in, node_victim);
		}

	}

	set_dirty(page, operation);
	return page;
}


struct A1OutGhostNode * get_ghost_page(){
    struct A1OutGhostNode * ghost = NULL;
    if(free_ghost_nodes->size > 0){
        ghost = (struct A1OutGhostNode *) list_remove_tail(free_ghost_nodes)->content;
    }else{
        if (A1out->size != Kout){
             printf("\n[ERROR] 2Q A1out is not full but free_ghost_nodes is empty");
        }
        ghost = (struct A1OutGhostNode *) list_remove_tail(A1out)->content;
    }
    if(ghost == NULL){
        printf("\n[ERROR] 2Q ghost is NULL?");
    }
    return ghost;
}

void insert_ghost_page(struct A1OutGhostNode * ghost ){
    
    if (A1out->size >= Kout){
        printf("\n[ERROR] 2Q A1out is full and there is an extra ghost");
    }
    list_insert_node_head(A1out, ghost->node);
}


void insert_MRU(struct List * list, struct Node  * node){
	list_insert_node_head(list, node);
}

struct Node  * remove_LRU(struct List * list){
	return list_remove_tail(list);
}


struct A1OutGhostNode * find_ghost_node(struct List * list, int file_id, int block_id){
    struct Node * x = list->head;
        while(x != NULL){
            struct A1OutGhostNode * ghost = (struct A1OutGhostNode *) x->content;
            if(ghost->file_id == file_id && ghost->block_id == block_id){
                return ghost;
            }
            x = x->next;
        }
    return NULL;
}



void buffer_print_policy(){
    struct Node * x = A1in->head;
    printf("\nA1in <-");
    while(x!=NULL){
        struct Page * n = (struct Page *) x->content;
        printf(" [%d-%ld]", n->file_id, n->block_id);
        x = x->next;
    }
    printf(" ->");
    
    x = A1out->head;
    printf(" A1out<-");
    while(x!=NULL){
        struct A1OutGhostNode * n = (struct A1OutGhostNode *) x->content;
        printf(" [%d-%ld]", n->file_id, n->block_id);
        x = x->next;
    }
    printf(" ->");

    x = Am->head;
    printf(" Am <-");
    while(x!=NULL){
        struct Page * n = (struct Page *) x->content;
        printf(" [%d-%ld]", n->file_id, n->block_id);
        x = x->next;
    }
    printf(" ->");

    x = free_ghost_nodes->head;
    printf("\nFree<-");
    while(x!=NULL){
        struct A1OutGhostNode * n = (struct A1OutGhostNode *) x->content;
        printf(" [%d-%ld]", n->file_id, n->block_id);
        x = x->next;
    }
    printf(" ->");

}


#endif
