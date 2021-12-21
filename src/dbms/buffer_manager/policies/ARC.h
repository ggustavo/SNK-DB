/*
 * Adaptive Replacement Cache (ARC) 
 * 
 * Reference Papers:
 *  - Outperforming LRU with an Adaptive Replacement Cache Algorithm (2004)
 *  - ARC: A Self-tuning, low Overhead Replacement Cache (2003)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define SAFE_DIVISION(a,b) ( ((b)==(0))?(0):((a)/(b)) ) 

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

struct List * T1;
struct List * T2;

struct List * G1; /* Ghost List G1 */
struct List * G2; /* Ghost List G2 */

int P; /*  Policy Advisor */
                                           
struct Ghost_Page{ /* Ghost page does not store any page data only its metadata */
    int file_id;
    long block_id;
};


struct Node * replacement(int x_E_G2, struct Node * ghost_node);
struct Ghost_Page * create_ghost_page();
void insert_MRU(struct List * list, struct Node * node);
struct Node * remove_LRU(struct List * list);
void move_to_MRU(struct List * source, struct Node * node, struct List * target);
struct Node * find_ghost_page(struct List * list, int file_id, int block_id);


/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){

    T1 = list_create(NULL,NULL);
    T2 = list_create(NULL,NULL);
    G1 = list_create(NULL,NULL);
    G2 = list_create(NULL,NULL);
    P = BUFFER_SIZE / 2;
    
    printf("\nBuffer Replacement Policy: %s \ninitial Advisor: %d", __FILE__, P);
    printf("\n---------------------------------------------------------------------------------------------------");

}

int L1(){
    return G1->size + T1->size;
}

int L2(){
    return G2->size + T2->size;
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------


	if(page != NULL){ /* HIT! x ∈ T1 or T2. Move x to the top of T2. */
        
        struct Node * node = (struct Node *)page->extended_attributes;
		
        if(node->list == T1){
            move_to_MRU(T1, node, T2); /* moves the page from T1 to T2 MRU */
        }  else  

        if(node->list == T2){
            move_to_MRU(T2, node, T2); /* moves the page from T2 to T2 MRU */
        }  else {
            printf("\n[ERR0] Why the page is not in T1 or T2 ???");
        } 

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

        struct Node * ghost_node = NULL;
        struct Node * victim = NULL;
        /* It is possible to have a page in G1 or G2 */
        
        ghost_node = find_ghost_page(G1, file_id, block_id);
        
        if(ghost_node != NULL){ /* Ghost HIT int G1 */
           
            P = MIN( BUFFER_SIZE, P + MAX( SAFE_DIVISION( G2->size, G1->size), 1) ); /* Adapt p = min{ buffer_size, p + max{ B2.size/B1.size, 1} } */
             
            list_remove(G1, ghost_node);
            victim = replacement(0, ghost_node); 
            page = (struct Page*) victim->content;
            buffer_load_page(file_id, block_id, page);
            insert_MRU(T2, victim);
            
            set_dirty(page, operation);
            return page;
    
        }
    
        ghost_node = find_ghost_page(G2, file_id, block_id);
        if(ghost_node != NULL){ /* Ghost HIT int G2 */
            
            P = MAX( BUFFER_SIZE, P - MAX( SAFE_DIVISION( G1->size, G2->size), 1) ); /* Adapt p = max{ 0, p - max{ B1.size/B2.size, 1} } */
            
            list_remove(G2, ghost_node);
            victim = replacement(1, ghost_node); /* X E G2 == 1 */
            page = (struct Page*) victim->content;
            buffer_load_page(file_id, block_id, page);
            insert_MRU(T2, victim);
            
            set_dirty(page, operation);
            return page;
        
        }

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
            struct Node * new_node = list_create_node(page); 
            buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
			insert_MRU(T1, new_node);

		} else {  /* Need a replacement */
            
            
            if( L1() == BUFFER_SIZE){

                if(T1->size < BUFFER_SIZE){ /* then delete the LRU page of G1 and REPLACE(p). */
                    ghost_node = remove_LRU(G1);
                    victim = replacement(0, ghost_node); 
                }else{ /* delete LRU page of T1 and remove it from the cache. */
                    
                    victim = remove_LRU(T1);
                    page = (struct Page*) victim->content;
			        buffer_flush_page(page);
                }

            }else 

            if(L1() < BUFFER_SIZE && (L1() + L2()) >= BUFFER_SIZE){ /* |L1| < c and |L1|+ |L2| ≥ c: */
                if(L1() + L2() == BUFFER_SIZE * 2){
                    ghost_node = remove_LRU(G2);
                }
                if(ghost_node == NULL){
                    ghost_node =  list_create_node(create_ghost_page());
                } 
                    
                victim = replacement(0, ghost_node);
            }

            page = (struct Page*) victim->content;
            buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            insert_MRU(T1, victim);
		}

	}
	set_dirty(page, operation);
	return page;
}

struct Node * replacement(int x_E_G2, struct Node * ghost_node){  
    struct Node * victim = NULL;
    struct Page * page = NULL;
    struct Ghost_Page * g_page = NULL;
    if(T1->size >= 1 && ( (x_E_G2 == 1 && T1->size == P) || T1->size > P)) { /* if (|T1| ≥ 1) and ((x ∈ G2 and |T1| = p) or (|T1| > p)) */
        victim = remove_LRU(T1);
        page = (struct Page * ) victim->content;
        g_page = (struct Ghost_Page *) ghost_node->content; 
        g_page->file_id  = page->file_id;
        g_page->block_id = page->block_id;
        insert_MRU(G1, ghost_node);
    }else{
        victim = remove_LRU(T2);
        page = (struct Page * ) victim->content;
        g_page = (struct Ghost_Page *) ghost_node->content; 
        g_page->file_id  = page->file_id;
        g_page->block_id = page->block_id;
        insert_MRU(G2, ghost_node);
    }
    buffer_flush_page(page);
    return victim;
}


struct Ghost_Page * create_ghost_page(){
    struct Ghost_Page * v_page = (struct Ghost_Page *) malloc (sizeof( struct Ghost_Page));
    //v_page->file_id = page->file_id;
    //v_page->block_id = page->block_id;
    return v_page;
}

void insert_MRU(struct List * list, struct Node * node){
    ((struct Page*)node->content)->extended_attributes = node;
	list_insert_node_head(list,node);
}

struct Node * remove_LRU(struct List * list){
    return list_remove_tail(list);
}

void move_to_MRU(struct List * source,struct Node * node, struct List * target){
	list_remove(source, node);
	list_insert_node_head(target, node);
}


struct Node * find_ghost_page(struct List * list, int file_id, int block_id){
    struct Node * x = list->head;
        while(x != NULL){
            struct Ghost_Page * ghost = (struct Ghost_Page *) x->content;
            if(ghost->file_id == file_id && ghost->block_id == block_id){
                return x;
            }
            x = x->next;
        }
    return NULL;
}

void buffer_print_policy(){
    struct Node * x = G1->tail;
    printf("\n<-");
    while(x!=NULL){
        struct Ghost_Page * page = (struct Ghost_Page *) x->content;
        printf(" [%d-%ld]", page->file_id, page->block_id);
        x = x->prev;
    }
    printf(" ->");
    x = T1->tail;
    printf(" <-");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" [%d-%ld]", page->file_id, page->block_id);
        x = x->prev;
    }
    printf(" ->");
    x = T2->head;
    printf(" <-");
    while(x!=NULL){
        struct Page * page = (struct Page *) x->content;
        printf(" [%d-%ld]", page->file_id, page->block_id);
        x = x->next;
    }
    printf(" ->");
    x = G2->head;
    printf(" <-");
    while(x!=NULL){
        struct Ghost_Page * page = (struct Ghost_Page *) x->content;
        printf(" [%d-%ld]", page->file_id, page->block_id);
        x = x->next;
    }
    printf(" ->");


}

#endif
