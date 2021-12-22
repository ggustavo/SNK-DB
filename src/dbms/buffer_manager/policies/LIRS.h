/*
 * Low Inter-reference Recency Set (LIRS)
 * 
 * Reference Paper:
 * - LIRS: An efficient low inter-reference recency set replacement policy to improve buffer cache performance (2002)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#define LIR 0
#define HIR 1

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"



struct List * stack_s;
struct List * list_q;

int LIR_size;
int HIR_size;

struct LIRSNode{
    struct Page * page;
    struct Node * node_stack_s;
    struct Node * node_list_q;
    int status;
    int is_ghost;
    int g_file_id;
    int g_block_id;
};


struct LIRSNode * LIRS_create_node(struct Page * page);
struct LIRSNode * get_victim();
void stack_pruning();
struct LIRSNode * find_ghost(int file_id, int block_id);
void print_LIRS_page(struct LIRSNode * node);
/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    //LIR_size = BUFFER_SIZE * 0.99; // 99%
    //HIR_size = BUFFER_SIZE - LIR_size; // 1%
    LIR_size = BUFFER_SIZE - 2; 
    HIR_size = BUFFER_SIZE - LIR_size; 
    printf("\nBuffer Replacement Policy: %s \nLIRs: %d HIRs: %d", __FILE__, LIR_size, HIR_size);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    stack_s = list_create(NULL,NULL);
    list_q =  list_create(NULL,NULL);
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update reference */

		struct LIRSNode * lirs_node = (struct LIRSNode *) page->extended_attributes;
        
        if(lirs_node->is_ghost == TRUE){
            
            printf("\n[ERR0R] Ghost page found in Real Buffer");
            exit(1);
        }

        if(lirs_node->node_stack_s != NULL){ //Hits in stack S
            
            list_remove(stack_s, lirs_node->node_stack_s);
            list_insert_node_head(stack_s, lirs_node->node_stack_s); //Move to top of stack
            
            stack_pruning();

            if(lirs_node->node_list_q != NULL){ //Hits in list Q
         
                if (lirs_node->status == LIR){
                    printf("\n[ERR0] LIR node in list Q");
                    exit(1);
                }
                list_remove(list_q, lirs_node->node_list_q); //removes the copy from list Q
                list_free_node(list_q, lirs_node->node_list_q);
                lirs_node->node_list_q = NULL;
                lirs_node->status = LIR; //promoted to LIR status

                //demote the LIR page at the bottom of the stack S to HIR status
                struct LIRSNode * demoted = (struct LIRSNode *) stack_s->tail->content; 

                if (demoted->status == HIR || demoted->node_list_q != NULL){
                    printf("\n[ERR0] HIR node in the bottom of the stack S");
                    exit(1);
                }

                demoted->status = HIR;
                demoted->node_list_q = list_remove(stack_s, demoted->node_stack_s);
                demoted->node_stack_s = NULL;
                list_insert_node_head(list_q, demoted->node_list_q);
                
                stack_pruning();
            }

        }

        else if(lirs_node->node_list_q != NULL){ // Hits on HIR page on list Q and not residing on stack S
           
            lirs_node->node_stack_s = list_create_node(lirs_node);
            lirs_node->status = HIR; //The HIR status of the page is kept
            //A new page entity is inserted in the top of stack S
            list_insert_node_head(stack_s, lirs_node->node_stack_s);
            
            //Move the page to the Head of list Q
            list_remove(list_q, lirs_node->node_list_q);
            list_insert_node_head(list_q, lirs_node->node_list_q);

        }else{
            printf("\n[ERR0] node not in stack S or list Q");
            exit(1);
        } 

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
			
            struct LIRSNode * lirs_node = LIRS_create_node(page);
            //New pages are inserted on top of stack S with LIR status
            if (stack_s->size < LIR_size) { 
                lirs_node->node_stack_s = list_create_node(lirs_node);
                lirs_node->status = LIR;
                list_insert_node_head(stack_s, lirs_node->node_stack_s);

            }else{ // When LIRs (LIR size) are reached
                lirs_node->node_stack_s = list_create_node(lirs_node);
                lirs_node->node_list_q  = list_create_node(lirs_node);
                lirs_node->status = HIR;
                // new pages are inserted with HIR status on top of stack S
                list_insert_node_head(stack_s, lirs_node->node_stack_s);
                //and head of list Q simultaneously
                list_insert_node_head(list_q, lirs_node->node_list_q);
            } 
            

		} else { /*  Need a replacement */
           
            struct LIRSNode * ghost = find_ghost(file_id, block_id);
            
            //printf("\n  ---- ");
            struct LIRSNode * node_victim = get_victim(); // Tail page of list Q as the victim
            struct Page * victim = node_victim->page;
            

            list_remove(list_q, node_victim->node_list_q);

            if (node_victim->node_stack_s != NULL) { // If this page also resides in stack S
                node_victim->status = HIR;
                node_victim->is_ghost = TRUE; //keeps only the metadata converting it into a ghost HIR page
                node_victim->g_file_id = node_victim->page->file_id;
                node_victim->g_block_id = node_victim->page->block_id;
                node_victim->page = NULL;
                list_free_node(list_q, node_victim->node_list_q);
                node_victim->node_list_q = NULL;
            }else{
                list_free_node(list_q, node_victim->node_list_q);
                node_victim->node_list_q = NULL;
                node_victim->page = NULL;
                free(node_victim);
            }

    

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            
            struct LIRSNode * new_node = NULL;
            
            if(ghost != NULL){
               
                new_node = ghost;
                new_node->status = LIR;   
                new_node->is_ghost = FALSE;
                list_remove(stack_s, new_node->node_stack_s);
                new_node->node_list_q  = NULL;
                list_insert_node_head(stack_s, new_node->node_stack_s);
                new_node->page = page;
                page->extended_attributes = new_node;

                //demote the LIR page at the bottom of the stack S to HIR status
                struct LIRSNode * demoted = (struct LIRSNode *) stack_s->tail->content; 

                if (demoted->status == HIR || demoted->node_list_q != NULL){
                    printf("\n[ERR0] HIR node in the bottom of the stack S");
                    exit(1);
                }

                demoted->status = HIR;
                demoted->node_list_q = list_remove(stack_s, demoted->node_stack_s);
                demoted->node_stack_s = NULL;
                list_insert_node_head(list_q, demoted->node_list_q);


            }else{
                new_node = LIRS_create_node(page);  
                new_node->node_stack_s = list_create_node(new_node);
                new_node->node_list_q  = list_create_node(new_node);
                new_node->status = HIR;    
                list_insert_node_head(stack_s, new_node->node_stack_s);
                list_insert_node_head(list_q, new_node->node_list_q);
            }
            
            
            if(new_node->page == NULL){
                printf("\n[ERR0] new_node->page == NULL");
                exit(1);
            }

            stack_pruning();
		}

	}
	set_dirty(page, operation);
	return page;
}

struct LIRSNode * get_victim(){
    
    struct Node * node = list_q->tail;
    struct LIRSNode * victim = ((struct LIRSNode *) node->content);
    
    return victim;
}

struct LIRSNode * find_ghost(int file_id, int block_id){
    struct Node * x = stack_s->head;
    while(x!=NULL){
        struct LIRSNode * lirs_node = (struct LIRSNode *) x->content;
        if (lirs_node->is_ghost == TRUE && lirs_node->node_list_q != NULL){
            printf("\n[ERR0] ghost node in the list Q");
            exit(1);
        }
        if (lirs_node->g_file_id == file_id && lirs_node->g_block_id == block_id){
            if (lirs_node->is_ghost == FALSE){
                printf("\n[ERR0] Hitting a ghost page without ghost status");
                exit(1);
            }
            return lirs_node;
        }
        x = x->next;
    }
    return NULL;
}

void stack_pruning(){
    struct Node * x = stack_s->tail;
    struct LIRSNode * lirs_node = (struct LIRSNode *) x->content;
    if(lirs_node->status == HIR){
        list_remove(stack_s, x);
        list_free_node(stack_s, x);
        lirs_node->node_stack_s = NULL;

        debug("\nstack_pruning ");
        //print_LIRS_page(lirs_node);
        
        if(lirs_node->is_ghost == TRUE){
            if(lirs_node->page != NULL){
                printf("\n[ERR0] ghost node with no NULL page");
                exit(1);
            }
            free(lirs_node);
        }

        stack_pruning();
    }

}

struct LIRSNode * LIRS_create_node(struct Page * page){
    struct LIRSNode * lirs_node = (struct LIRSNode *) malloc (sizeof(struct LIRSNode));
    
    lirs_node->node_list_q = NULL;
    lirs_node->node_stack_s = NULL;
    lirs_node->page = page;
    lirs_node->status = -1; 
    lirs_node->is_ghost = FALSE;

    lirs_node->g_file_id = -1;
    lirs_node->g_block_id = -1;

    page->extended_attributes = lirs_node;
    return lirs_node;
}

void buffer_print_policy(){
  
    struct Node * x = stack_s->head;
    printf("\nS->");
    while(x!=NULL){
        struct LIRSNode * lirs_node = (struct LIRSNode *) x->content;
        print_LIRS_page(lirs_node);
        x = x->next;
    }

    printf(" Q->");
    x = list_q->head;
    while(x!=NULL){
        struct LIRSNode * lirs_node = (struct LIRSNode *) x->content;
        print_LIRS_page(lirs_node);
        x = x->next;
    }
    printf(" ->");
    
}

void print_LIRS_page(struct LIRSNode * node){
    if(node == NULL){
        printf("\n[ERR0] Printing a NULL node");
        return;
    }
    if(node->status == LIR){
        if(node->is_ghost == TRUE){
            printf(" [%d-%ld]L*(ERR0)", node->page->file_id, node->page->block_id); 
        }else{
            printf(" [%d-%ld]L", node->page->file_id, node->page->block_id);
        }
    }else
    if(node->status == HIR){
        if(node->is_ghost == TRUE){
            printf(" [%d-%ld]H*", node->g_file_id, node->g_block_id);
        }else{
            printf(" [%d-%ld]H", node->page->file_id, node->page->block_id);
        }

    }else{
        printf("\n[ERR0] status Page error [%d-%ld]?", node->page->file_id, node->page->block_id);
        exit(1);
    }

}

#endif
