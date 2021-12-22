/*
 * Frequency-Based Replacement (FBR)
 * Simplified algorithm without index and the parameters Amax and Cmax 
 * 
 *  Reference Paper: 
 *  - Data cache management using frequency-based replacement
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- FBR Structure --
* 
*                    void *extended_attributes                             struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct FBRNode  |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/


struct FBRNode{
    struct Page * page;
    struct Node * node;
    int reference;
};


struct List * new_section;
struct List * mid_section;
struct List * old_section;

int F_NEW;
int F_MID;
int F_OLD;


struct FBRNode * FBR_create_node(struct Page * page);
void FBR_insert_new(struct FBRNode * fbr_node);
void FBR_insert_mid(struct FBRNode * fbr_node);
void FBR_insert_old(struct FBRNode * fbr_node);
struct FBRNode * get_victim();

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */

void buffer_policy_start(){

    new_section = list_create(NULL,NULL);
    mid_section = list_create(NULL,NULL);
    old_section = list_create(NULL,NULL);
    F_NEW = BUFFER_SIZE / 4;
    F_OLD = BUFFER_SIZE / 4;
    
    F_MID = BUFFER_SIZE - (F_NEW + F_OLD);
    
    printf("\nBuffer Replacement Policy: %s \nF_NEW: %d / F_MID: %d / F_OLD: %d", __FILE__, F_NEW,F_MID,F_OLD);
    printf("\n---------------------------------------------------------------------------------------------------");

}


struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - */

		struct FBRNode * fbr_node = (struct FBRNode *) page->extended_attributes;
        struct List * list = fbr_node->node->list;
        list_remove(list, fbr_node->node);
        
        if(list != NULL && list != new_section){
            fbr_node->reference =  fbr_node->reference  + 1;          
        }

        FBR_insert_new(fbr_node);

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct FBRNode * new_node = FBR_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            FBR_insert_new(new_node);

		} else { /*  Need a replacement */


            struct FBRNode * node_victim = get_victim();
            
            struct Page * victim = node_victim->page;

            printf("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->reference);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            node_victim->reference = 1;
            FBR_insert_new(node_victim);
		}

	}
	set_dirty(page, operation);
	return page;
}

struct FBRNode * get_victim(){
    
    struct Node * node = old_section->tail;
    struct FBRNode * victim = ((struct FBRNode *) node->content);
    
    int min = victim->reference;
    int reference = -1;

    while(node != NULL){
        reference = ((struct FBRNode *) node->content)->reference;
        
        if(reference < min){
            min = reference;
            victim = ((struct FBRNode *) node->content);
        }
        
        node = node->prev;
    }

    list_remove(old_section, victim->node);
    return victim;
}


void FBR_insert_new(struct FBRNode * fbr_node){

    if(new_section->size >= F_NEW){
        struct Node * node = list_remove_tail(new_section);
        FBR_insert_mid( (struct FBRNode *) node->content);
    }

	list_insert_node_head(new_section, fbr_node->node);

}

void FBR_insert_mid(struct FBRNode * fbr_node){

    if(mid_section->size >= F_MID){
        struct Node * node = list_remove_tail(mid_section);
        FBR_insert_old( (struct FBRNode *) node->content);
    }

	list_insert_node_head(mid_section, fbr_node->node);
}


void FBR_insert_old(struct FBRNode * fbr_node){

    if(old_section->size >= F_OLD){
        printf("\n[ERR0] Inserting in the old section with maximum size already reached");
    }

	list_insert_node_head(old_section, fbr_node->node);
}

struct FBRNode * FBR_create_node(struct Page * page){
    struct FBRNode * fbr_node = (struct FBRNode *) malloc (sizeof(struct FBRNode));
    
    struct Node * new_node = list_create_node(fbr_node);
    
    fbr_node->reference = 1;
    fbr_node->node = new_node;
    fbr_node->page = page;   

    page->extended_attributes = fbr_node;
    return fbr_node;
}


void buffer_print_policy(){
    struct Node * x = new_section->head;
    printf("\n<-");
    while(x!=NULL){
        struct FBRNode * n = (struct FBRNode *) x->content;
        printf(" [%d-%ld]:%d", n->page->file_id, n->page->block_id, n->reference);
        x = x->next;
    }
    printf(" ->");
    x = mid_section->head;
    while(x!=NULL){
        struct FBRNode * n = (struct FBRNode *) x->content;
        printf(" [%d-%ld]:%d", n->page->file_id, n->page->block_id, n->reference);
        x = x->next;
    }
    printf(" ->");
    x = old_section->head;
    while(x!=NULL){
        struct FBRNode * n = (struct FBRNode *) x->content;
        printf(" [%d-%ld]:%d", n->page->file_id, n->page->block_id, n->reference);
        x = x->next;
    }
   

}

#endif
