/*
 * Multi queue (MQ)
 * 
 * Reference Papers: 
 *  - Yuanyuan Zhou, Memory Management for Networked Servers (2000)
 *  - The Multi-Queue Replacement Algorithm for Second Level Buffer Caches (2001)
 */

#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"
#include <math.h> // to use LOG function
#define MIN(a,b) (((a)<(b))?(a):(b))

unsigned long long int currentTime;
unsigned long long int lifeTime;

int M; /*  number of queues! */

struct List ** Queues;
struct List * Qout;

int Qout_size;

struct MQNode{
    struct Page * page;
    struct Node * node;
    int reference;
    unsigned long long int expireTime;
};

struct MQGhostNode{
    struct Node * node;
    int file_id;
    long block_id;
    int reference;
};

int QueueNum(int reference);
void Adjust();
struct MQNode * MQ_create_node(struct Page * page);
void insert_MRU(struct List * list, struct Node  * node);
struct Node  * remove_LRU(struct List * list);
struct MQGhostNode * create_ghost_page();
struct MQGhostNode * find_ghost_node(struct List * list, int file_id, int block_id);
struct MQNode * EvictBlock();

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    currentTime = 0;
    lifeTime = BUFFER_SIZE / 2;
    M = 8;
    Qout_size = BUFFER_SIZE * 4; /*  The history buffer Qout size is set to be four time of the number of blocks in the cache */

    printf("\nBuffer Replacement Policy: %s with M %d queues", __FILE__, M);
    printf("\n---------------------------------------------------------------------------------------------------");
    
    Qout = list_create(buffer_print_page,NULL);
    
    Queues = (struct List **)malloc(sizeof( struct List *) * M); 
    for(int k = 0; k < M; k++){
        Queues[k] = list_create(buffer_print_page,NULL);
    }
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update reference */

		struct MQNode * B = (struct MQNode *) page->extended_attributes;
        B->reference = B->reference + 1;
        B->expireTime = currentTime + lifeTime;
        struct List * B_queue = B->node->list;
        struct List * K_queue = Queues[ QueueNum(B->reference) ];
      
        insert_MRU(K_queue, list_remove(B_queue, B->node));  /* Moves B from B_queue to K_queue */

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */


        struct MQGhostNode * ghost = find_ghost_node(Qout, file_id, block_id);
        
        if(ghost != NULL){ /* GHOST HIT !! */
            
            int references = ghost->reference;
            
            struct MQNode * victim = EvictBlock(); 
            
            list_remove(Qout, ghost->node);           /* Removes hitted ghost */
            /* We reuse the hitted ghost structure and update the victim's information on it */
            ghost->file_id  = victim->page->file_id;  /* Update file_id */
            ghost->block_id = victim->page->block_id; /* Update block_id */
            ghost->reference = victim->reference;     /* Udate reference */
            insert_MRU(Qout, ghost->node);            /* Insert "new Ghost" */ 
           

            /* We reuse the victim structure */
            page = buffer_reset_page(victim->page);
            buffer_load_page(file_id, block_id, page);   /* Read the data from storage media */
            victim->reference = references;              /* Don't forget the hitted ghost reference counter */
            victim->expireTime = currentTime + lifeTime;
            struct List * K_queue = Queues[ QueueNum(victim->reference) ]; 
            insert_MRU(K_queue, victim->node); /* inserts into a K queue */

        }else


		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
            buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
			
			struct MQNode * B = MQ_create_node(page);  
            B->reference = B->reference + 1;
            B->expireTime = currentTime + lifeTime;

            struct List * K_queue = Queues[ QueueNum(B->reference) ];
            insert_MRU(K_queue, B->node);
           

		} else { /* Need a replacement */


            struct MQNode * victim = EvictBlock(); 

            if(Qout->size < Qout_size){
                ghost = create_ghost_page();
            }else{
                ghost = (struct MQGhostNode *) remove_LRU(Qout)->content;
            }

            ghost->file_id  = victim->page->file_id;  /* Update file_id */
            ghost->block_id = victim->page->block_id; /* Update block_id */
            ghost->reference = victim->reference;     /* Udate reference */
            insert_MRU(Qout, ghost->node);            /* Insert "new Ghost" */

            
            page = victim->page;
			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */

            victim->reference = 1;
            victim->expireTime = currentTime + lifeTime;
            struct List * K_queue = Queues[ QueueNum(victim->reference) ]; 
            insert_MRU(K_queue, victim->node); /* inserts into a K queue */
            
		}

	}
    Adjust();
	set_dirty(page, operation);
	return page;
}


struct MQNode * EvictBlock(){
    struct MQNode * victim = NULL;
    
    for(int i = 0; i < M; i++){
        struct List * queue = Queues[i];
        if(queue->size > 0){
            victim = (struct MQNode *) remove_LRU(queue)->content;
            break;
        }
    }
    printf("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->page->dirty_flag, victim->page->file_id, victim->page->block_id);
    buffer_flush_page(victim->page); /* Flush the data to the secondary storage media if is dirty */
    return victim;
}   

int QueueNum(int reference){
    
    int queue = MIN( log2(reference), M - 1 );

    printf("\n --> Sent to List: %d", queue);
    return queue;
}

void Adjust(){
     currentTime = currentTime + 1;
    for(int i = 1; i < M; i++){ // Start with i = 1
        struct List * queue = Queues[i];
             if(queue->size > 0){   
                struct MQNode * LRU = (struct MQNode *)  queue->tail->content;
                if(LRU->expireTime < currentTime){
                    printf("\nAjust %c[%d-%d]", LRU->page->dirty_flag, LRU->page->file_id, LRU->page->block_id );
                    list_remove(queue, LRU->node);
                    insert_MRU(Queues[i - 1], LRU->node);
                    LRU->expireTime = currentTime + lifeTime;
                }
             }
        }
}

struct MQNode * MQ_create_node(struct Page * page){
    struct MQNode * MQ_node = (struct MQNode *) malloc (sizeof(struct MQNode));
    
    struct Node * new_node = list_create_node(MQ_node);
    
    MQ_node->node = new_node;
    MQ_node->page = page;   
    MQ_node->reference = 0;
    MQ_node->expireTime = 0;
    page->extended_attributes = MQ_node;
    return MQ_node;
}

void insert_MRU(struct List * list, struct Node  * node){
	list_insert_node_head(list, node);
}

struct Node  * remove_LRU(struct List * list){
	return list_remove_tail(list);
}

struct MQGhostNode * create_ghost_page(){
    struct MQGhostNode * ghost = (struct MQGhostNode *) malloc (sizeof( struct MQGhostNode));
    ghost->node = list_create_node(ghost);
    return ghost;
}

struct MQGhostNode * find_ghost_node(struct List * list, int file_id, int block_id){
    struct Node * x = list->head;
        while(x != NULL){
            struct MQGhostNode * ghost = (struct MQGhostNode *) x->content;
            if(ghost->file_id == file_id && ghost->block_id == block_id){
                return ghost;
            }
            x = x->next;
        }
    return NULL;
}



void buffer_print_policy(){
    
    for(int i = 0; i < M; i++){
        struct List * queue = Queues[i];
        struct Node * x = queue->head;
        printf("\n%d <-", i);
        while(x!=NULL){
            struct MQNode * node = (struct MQNode *) x->content;
            printf(" [%d-%ld (R:%d E:%d)]", node->page->file_id, node->page->block_id, node->reference, node->expireTime);
            x = x->next;
        }
        printf(" ->");
    }

    struct Node * x = Qout->head;
    printf("\nG <-");
    while(x != NULL){
        struct MQGhostNode * node = (struct MQGhostNode *) x->content;
        printf(" [%d-%ld (R:%d)]", node->file_id, node->block_id, node->reference);
         x = x->next;;
    }
    printf(" ->");

}
#endif
