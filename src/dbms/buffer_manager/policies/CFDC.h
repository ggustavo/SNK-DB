/*
 * Clean-First Dirty-Clustered (CFDC)
 * Reference Paper:
 * - CFDC: A flash-aware replacement policy for database buffer management (2009)
 * - CFDC: A Flash-Aware Buffer Management Algorithm for Database Systems(2010)
 * 
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED


#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

struct List * working_queue;
struct List * clean_queue;
struct List * dirty_queue;

int working_region_size;
int priority_region_size;
int GC; //global time
int MAX_CLUSTER_SIZE;    


struct Cluster{
    int id;
    double priority;
    int timestamp;
    struct List * list;
    struct Node * node;
};

struct CFDCNode{
    struct Page * page;
    struct Node * node;
    struct Cluster * cluster;
};

struct CFDCNode * CFDC_create_node(struct Page * page);
struct CFDCNode * get_dirty_victim();
void CFDC_insert(struct List * list, struct CFDCNode * node);
void CFDC_insert_working(struct CFDCNode * cfdc_node);
void CFDC_insert_priority(struct CFDCNode * cfdc_node);
void CFDC_insert_dirty(struct CFDCNode * cfdc_node);
int CFDC_page_id(struct Page * page);
int CFDC_cluster_id(struct Page * page);
struct Cluster* CFDC_target_cluster(struct Page * x);
struct CFDCNode * CFDC_remove_from_cluster(struct CFDCNode * cfdc_node);
void CFDC_destroy_cluster(struct Cluster * cluster);
double CFDC_priority_function(struct Cluster * cluster);
struct Cluster * CFDC_create_cluster(int id);

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    working_region_size = BUFFER_SIZE / 2;
    priority_region_size = BUFFER_SIZE - working_region_size;
    GC = 0;
    MAX_CLUSTER_SIZE = 16; // 16, 64

    printf("\nBuffer Replacement Policy: %s\nWorking Region: %d Priority Region: %d Elements per Cluster: %d", __FILE__, working_region_size ,priority_region_size ,MAX_CLUSTER_SIZE);
    printf("\n---------------------------------------------------------------------------------------------------");
	
    working_queue = list_create(NULL,NULL);
    clean_queue = list_create(NULL,NULL);
    dirty_queue = list_create(NULL,NULL);
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

	if(page != NULL){ /* HIT - Update reference */

        
		struct CFDCNode * cfdc_node = (struct CFDCNode *) page->extended_attributes;
       
        // page is moved to MRU working region
        if(cfdc_node->node->list == working_queue){
            list_remove(working_queue, cfdc_node->node);
            CFDC_insert_working(cfdc_node);
        }else if (cfdc_node->node->list == clean_queue){
            list_remove(clean_queue, cfdc_node->node);
            CFDC_insert_working(cfdc_node);
        }else{
            if(cfdc_node->cluster == NULL){
                printf("\n[ERR0] CFDC: page is not in any cluster or list");
                exit(1);
            }
            if (cfdc_node->node->list != cfdc_node->cluster->list){
                printf("\n[ERR0] CFDC: node list is not in the same cluster list");
            }

            /*
            If p is to be promoted due to a buffer hit, we update
            its cluster IPD including the timestamp
            */
            struct Cluster * cluster = cfdc_node->cluster;
            CFDC_remove_from_cluster(cfdc_node);
            CFDC_insert_working(cfdc_node);

            if(cluster->list->size == 0){
                list_remove(dirty_queue, cluster->node);
                CFDC_destroy_cluster(cluster);
            }else{
                cluster->timestamp = GC;
                cluster->priority = CFDC_priority_function(cluster);
            }

        }

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct CFDCNode * new_node = CFDC_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            CFDC_insert_working(new_node);
           
		} else { /*  Need a replacement */


            struct CFDCNode * node_victim = NULL;

            if(clean_queue->size > 0){ //Clean pages are always selected over dirty pages
                node_victim = (struct  CFDCNode *) list_remove_tail(clean_queue)->content;
            }else{ //If there is no clean page available
                //A cluster having the lowest priority is selected from the priority queue of dirty
                node_victim = get_dirty_victim(); // the oldest unfixed page in this cluster is selected as victim
                
            }

            if (node_victim == NULL){
                printf("\nERROR: node_victim == NULL");
                exit(1);
            }
            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

            CFDC_insert_working(node_victim);
		}

	}
	set_dirty(page, operation);
	return page;
}


void CFDC_insert(struct List * list, struct CFDCNode * cfdc_node){
	list_insert_node_head(list, cfdc_node->node);
}

void CFDC_insert_working(struct CFDCNode * cfdc_node){
	if(working_queue->size >= working_region_size){
        // LRU page is removed from the working queue
        struct Node * lru = list_remove_tail(working_queue); 
        //debug("\n ---- LRU: %c[%d-%d]", ((struct CFDCNode *) lru->content)->page->dirty_flag, ((struct CFDCNode *) lru->content)->page->file_id, ((struct CFDCNode *) lru->content)->page->block_id);
        CFDC_insert_priority((struct CFDCNode *) lru->content);
    }
    CFDC_insert(working_queue, cfdc_node);
}

void CFDC_insert_priority(struct CFDCNode * cfdc_node){
    if(cfdc_node->page->dirty_flag == PAGE_CLEAN){
        CFDC_insert(clean_queue, cfdc_node);
    }else{
        CFDC_insert_dirty(cfdc_node);
    }
}

void CFDC_insert_dirty(struct CFDCNode * cfdc_node){
    struct Cluster * cluster = CFDC_target_cluster(cfdc_node->page);
    
    if(cluster == NULL){
        cluster = CFDC_create_cluster( CFDC_cluster_id(cfdc_node->page) );
        //The cluster timestamp is the value of globaltime at the time of its creation.
        cluster->timestamp = GC;
        list_insert_node_head(dirty_queue, cluster->node);
    }

    /*
      When adding demoted pages to a existing cluster, 
      the cluster timestamp is not updated.
    */
    cfdc_node->cluster = cluster;
    list_insert_node_head(cluster->list, cfdc_node->node);

    GC++; // Each time a dirty page is evicted from the working region, globaltime is incremented by 1.
   
    //priority queue is adjusted.
    cfdc_node->cluster->priority = CFDC_priority_function(cluster);
    //printf("  Priority: %d", cfdc_node->cluster->priority);
}



double CFDC_priority_function(struct Cluster * cluster){
    if(cluster->list->head == NULL){
        printf("\n[ERR0] CFDC: cluster->list->head == NULL");
        exit(1);
    }

    if (cluster->priority == 0){
        return 0;
    }

    struct Node * node = cluster->list->head->next; //second node
    
    /*
    inter-page distance (IPD), is used to distinguish between randomly accessed clusters 
    and sequentially accessed clusters (clusters with only one page are set to 1)
    */
    int IPD = 0;

    while(node != NULL){
        struct CFDCNode * cfdc_node = (struct CFDCNode *) node->content;
        int page_id = CFDC_page_id(cfdc_node->page);
        
        struct CFDCNode * prev_cfdc_node = (struct CFDCNode *) node->prev->content;
        int prev_page_id = CFDC_page_id(prev_cfdc_node->page);

        IPD = IPD + ABS(page_id - prev_page_id);

        node = node->next;
    }

    if(node == NULL){
        IPD = 1;
    }

    int n_2 = cluster->list->size * cluster->list->size; 
    double value = SAFE_DIVISION( (double) IPD,  n_2 * (GC - cluster->timestamp) );

    debug("---- %d / %d * (%d - %d) = %f", IPD, n_2, GC, cluster->timestamp,value);
    return value;

}


struct CFDCNode * get_dirty_victim(){
    /*
    Once a victim is selected from a cluster, its priority is set to 0 and will not be updated
    anymore, so that the next victims will still be evicted from this victim cluster,
    resulting in strong spatial locality of page evictions.
    */
    
    struct Node * node = dirty_queue->tail;
    
    struct Cluster * cluster = NULL;
    struct Cluster * cluster_victim = NULL;

    double min = -1;

    while(node != NULL){
        cluster = (struct Cluster *) node->content;
        
        if(cluster->list->size == 0){
            printf("\n[ERR0] CFDC: cluster->list->size = 0");
            exit(1);
        }
        if(cluster->priority < 0){
            printf("\n[ERR0] CFDC: cluster->priority < 0");
            exit(1);
        }

        if(cluster->priority < min || min == -1){
            min = cluster->priority;
            cluster_victim = cluster;
        }

        node = node->prev;
    }

    if(cluster_victim == NULL){
        printf("\n[ERR0] CFDC: cluster_victim == NULL");
        exit(1);
    }

    cluster_victim->priority = 0; // The cluster is marked
    
    struct Node * n_v = cluster_victim->list->tail;
    struct CFDCNode * victim = (struct CFDCNode*) n_v->content;


    if(victim == NULL){
        printf("\n[ERR0] victim is NULL");
        exit(1);
    }

   // printf("\n ---- VICTIM: %c[%d-%d]", victim->page->dirty_flag, victim->page->file_id, victim->page->block_id);
    
    CFDC_remove_from_cluster(victim);


    if(cluster_victim->list->size == 0){
        list_remove(dirty_queue, cluster_victim->node);
        CFDC_destroy_cluster(cluster_victim);
    }
    
    return victim;
}



struct Cluster * CFDC_create_cluster(int id){
    struct Cluster * cluster = (struct Cluster *) malloc(sizeof(struct Cluster));
    cluster->list = list_create(NULL,NULL);
    cluster->id = id;
    cluster->priority = -1;
    cluster->timestamp = -1; 
    cluster->node = list_create_node(cluster);
    return cluster;
}

struct CFDCNode * CFDC_create_node(struct Page * page){
    struct CFDCNode * cfdc_node = (struct CFDCNode *) malloc (sizeof(struct CFDCNode));
    
    struct Node * new_node = list_create_node(cfdc_node);
    
    cfdc_node->cluster = NULL;
    cfdc_node->node = new_node;
    cfdc_node->page = page;   

    page->extended_attributes = cfdc_node;
    return cfdc_node;
}


struct Cluster* CFDC_target_cluster(struct Page * x){
   
	int cluster_id = CFDC_cluster_id(x);

	struct Node * n = dirty_queue->head;
    struct Cluster * cluster = NULL;

	while(n != NULL){
        cluster = ((struct Cluster *) n->content);
        
        if(cluster->id == cluster_id){
            return cluster;
        }    
        n = n->next;
    }

	return NULL;

}

struct CFDCNode * CFDC_remove_from_cluster(struct CFDCNode * cfdc_node){
    struct Cluster * cluster = cfdc_node->cluster;
    if(cluster == NULL){
        printf("\n[ERR0] CFDC: node is not in any cluster");
        exit(1);
    }
    
    list_remove(cluster->list, cfdc_node->node);   
    cfdc_node->cluster = NULL;

    return cfdc_node;
}

void CFDC_destroy_cluster(struct Cluster * cluster){
   
    list_free_node(cluster->list, cluster->node);
    
    if (cluster->list != NULL && cluster->list->size > 0){
        printf("\n[ERR0] CFDC: cluster list is not empty");
        exit(1);
    }

    list_free(cluster->list);    
    
    free(cluster);
}

int CFDC_page_id(struct Page * page){
    //Use Cantor pairing function to get the cluster id
    //(a + b) * (a + b + 1) / 2 + a; where a, b >= 0
    //int page_id = (page->file_id + page->block_id) * (page->file_id + page->block_id + 1) / 2 + page->block_id;
    return page->block_id;
}

int CFDC_cluster_id(struct Page * page){

    return CFDC_page_id(page) / MAX_CLUSTER_SIZE;
}

void buffer_print_policy(){
    
    struct Node * x = working_queue->head;
    printf("\n<-");
    while(x!=NULL){
        struct CFDCNode * cfdc_node = (struct CFDCNode *) x->content;     
        if(cfdc_node==NULL){
            printf("\n[ERR0] CFDC PRINT: cfdc_node is NULL");
            exit(1);
        }
        struct Page * page = cfdc_node->page;
        if(page==NULL){
            printf("\n[ERR0] CFDC PRINT: page is NULL");
            exit(1);
        }
        
        printf(" %c[%d-%d]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }

    printf(" ->");
    x = clean_queue->head;
    while(x!=NULL){
        struct CFDCNode * cfdc_node = (struct CFDCNode *) x->content;
        struct Page * page = cfdc_node->page;
        printf(" %c[%d-%d]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
    
    x = dirty_queue->head;
    printf(" ->");
    if(dirty_queue->size > 0){
        
        while(x!=NULL){
            struct Cluster * cluster = (struct Cluster *) x->content;
            struct Node * n = cluster->list->head;
            printf(" P:%.3f;id:%d;T:%d(",cluster->priority, cluster->id, cluster->timestamp);
            //printf(" size:%d;", cluster->list->size);
            while(n!=NULL){
                struct CFDCNode * cfdc_node = (struct CFDCNode *) n->content;
                struct Page * page = cfdc_node->page;
                printf(" %c[%d-%d]", page->dirty_flag, page->file_id, page->block_id);
                n = n->next;
            }
        printf(")");
        x = x->next;
        }
    }
   
}

#endif
