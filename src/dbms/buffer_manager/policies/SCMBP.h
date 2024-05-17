/*
 * SCMBP-SCCW
 * Reference Paper:
 * - Database Buffer Management Strategies for Asymmetric Media (2015)
 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

int Di();
int Ci();
int Do();
int Co();
int C();
int D();


// --------------- Enlarge Magic Values -----------------
float Miss_Enlarger_C = 1.0;
float Miss_Enlarger_D = 4.0;

float Hit_Enlarger_C = 0.5;
float Hit_Enlarger_D = 2.0;

float Ghost_Hit_Enlarger_C = 1.0;
float Ghost_Hit_Enlarger_D = 4.0;


float Ghost_Hit_Enlarger_RecentClean = 0.5;
float Ghost_Hit_Enlarger_FrequentClean = 1.0;

float Ghost_Hit_Enlarger_RecentDirty = 0.5;
float Ghost_Hit_Enlarger_FrequentDirty = 1.0;

// --------------- Policy Advisor -----------------
float P;
float PD;
float PC;

int GC;
int MAX_CLUSTER_SIZE;

struct List * RECENT_CLEAN; //RecentClean
struct List * RECENT_DIRTY; // RecentDirty

struct List * FREQUENT_CLEAN; //FrequentClean
struct List * FREQUENT_DIRTY; // FrequentDirty

struct List * GHOST_RECENT_CLEAN; //GhostRecentClean
struct List * GHOST_RECENT_DIRTY; //GhostRecentDirty

struct List * GHOST_FREQUENT_CLEAN; //GhostFrequentClean
struct List * GHOST_FREQUENT_DIRTY; //GhostFrequentDirty

struct Cluster{
    int id;
    double priority;
    int timestamp;
    struct List * list;
    struct Node * node;
};

struct SCMBPNode{
    struct Page * page;
    struct Node * node;
	struct Cluster * cluster;

    int ghost_bit;
    int g_file_id;
    int g_block_id;
};

struct SCMBPNode * SCMBP_create_node(struct Page * page);
void SCMBP_destroy_ghost(struct SCMBPNode * ghost_node);
struct SCMBPNode * get_victim(char operation, struct SCMBPNode * ghost );
void SCMBP_insert_MRU(struct List * list, struct SCMBPNode * node);
void SCMBP_insert_new(struct SCMBPNode * new_node, char operation);
struct SCMBPNode * removePageC(struct SCMBPNode * ghost);
struct SCMBPNode * removePageD(struct SCMBPNode * ghost);
struct SCMBPNode * remove_ghost_lru(struct List * list, struct SCMBPNode * ghost);
void SCMBP_insert_ghost(struct List * list, struct SCMBPNode * ghost, struct SCMBPNode * node_victim);
void SCMBP_ajust_ghost_values(struct Node * x, char operation);
struct SCMBPNode * find_ghost(int file_id, int block_id);
struct SCMBPNode * find_ghost_list(struct List * list, int file_id, int block_id);
struct Node * select_ghost_victim();

int SCMBP_cluster_id(struct Page * page);
int SCMBP_page_id(struct Page * page);
void SCMBP_destroy_cluster(struct Cluster * cluster);
struct SCMBPNode * SCMBP_remove_from_cluster(struct SCMBPNode * scmbp_node);
struct Cluster* SCMBP_target_cluster(struct Page * x);
struct Cluster * SCMBP_create_cluster(int id);
double SCMBP_priority_function(struct Cluster * cluster);

void SCMBP_insert_cluster(struct SCMBPNode * scmbp_node);
struct SCMBPNode * get_cluster_victim();

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
	MAX_CLUSTER_SIZE = 16; // 16, 64
	GC = 0;
    printf("\nBuffer Replacement Policy: %s \nElements per Cluster: %d", __FILE__, MAX_CLUSTER_SIZE);
    printf("\n---------------------------------------------------------------------------------------------------");

    P =  (float) BUFFER_SIZE / 2;
	PD = (float) BUFFER_SIZE / 4;
	PC = (float) BUFFER_SIZE / 4;

    RECENT_CLEAN = list_create(NULL,NULL);
    RECENT_DIRTY = list_create(NULL,NULL);

    FREQUENT_CLEAN = list_create(NULL,NULL);
    FREQUENT_DIRTY = list_create(NULL,NULL);

    GHOST_FREQUENT_CLEAN = list_create(NULL,NULL);
    GHOST_FREQUENT_DIRTY = list_create(NULL,NULL);

    GHOST_RECENT_CLEAN = list_create(NULL,NULL);
    GHOST_RECENT_DIRTY = list_create(NULL,NULL);
}

int RECENT_DIRTY_size(){
	int RECENT_DIRTY_size = 0;
	if(RECENT_DIRTY->size > 0){
		struct Node * x = RECENT_DIRTY->tail;
        while(x!=NULL){
            struct Cluster * cluster = (struct Cluster *) x->content;
            struct Node * n = cluster->list->head;
   
            while(n != NULL){
                RECENT_DIRTY_size ++;
                n = n->next;
            }
        	x = x->prev;
        }
    }
	return RECENT_DIRTY_size;
}

int Di() {
	return RECENT_DIRTY_size() + FREQUENT_DIRTY->size;
}

int Ci() {
	return RECENT_CLEAN->size + FREQUENT_CLEAN->size;
}

int Do() {
	return GHOST_RECENT_DIRTY->size + GHOST_FREQUENT_DIRTY->size;
}

int Co() {
	return GHOST_RECENT_CLEAN->size + GHOST_FREQUENT_CLEAN->size;
}

int C() {
	return Ci() + Co();
}

int D() {
	return Di() + Do();
}

void assert(){
    if (Di() + Ci() > BUFFER_SIZE){
        printf("\nDi + Ci > BUFFER_SIZE");
        exit(1);
    }
    if (Do() + Co() > BUFFER_SIZE){
        printf("\nDo + Co > BUFFER_SIZE");
        exit(1);
    }
    if(C() + D() > BUFFER_SIZE * 2){
        printf("\nC + D > BUFFER_SIZE");
        exit(1);
    }
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){
    //assert();
	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------
	
	if(page != NULL){ /* HIT - Update MRU */
		
		set_dirty(page, operation);
		struct SCMBPNode * scmpb_node = (struct SCMBPNode *) page->extended_attributes;
		struct List * list_r = scmpb_node->node->list;

		if (list_r != RECENT_CLEAN && list_r != FREQUENT_CLEAN && list_r != FREQUENT_DIRTY){
			
			if(scmpb_node->cluster == NULL){
                printf("\n[ERR0] SCMBP: page is not in any cluster or list");
                exit(1);
            }
            if (scmpb_node->node->list != scmpb_node->cluster->list){
                printf("\n[ERR0] SCMBP: node list is not in the same cluster list");
				exit(1);
            }
			 /*
            If p is to be promoted due to a buffer hit, we update
            its cluster IPD including the timestamp
            */
	
            struct Cluster * cluster = scmpb_node->cluster;
            SCMBP_remove_from_cluster(scmpb_node);
           
            if(cluster->list->size == 0){
                list_remove(RECENT_DIRTY, cluster->node);
                SCMBP_destroy_cluster(cluster);
            }else{
                cluster->timestamp = GC;
                cluster->priority = SCMBP_priority_function(cluster);
            }
			

		}else{
			
			list_remove(scmpb_node->node->list, scmpb_node->node);
		}

		if(page->dirty_flag == PAGE_CLEAN){
			SCMBP_insert_MRU(FREQUENT_CLEAN, scmpb_node);
		}else{
			SCMBP_insert_MRU(FREQUENT_DIRTY, scmpb_node);
		}

	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */

		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct SCMBPNode * new_node = SCMBP_create_node(page);
			buffer_load_page(file_id, block_id, page); // Read the data from storage media
			
			SCMBP_insert_new(new_node, operation);

		} else { /* Need a replacement */
			
			struct SCMBPNode * ghost = find_ghost(file_id, block_id);
			if (ghost != NULL){
				SCMBP_ajust_ghost_values(ghost->node, operation);
			}

			struct SCMBPNode * node_victim = NULL;
            
			if (Ci() + Di() == BUFFER_SIZE) {
			    node_victim = get_victim(operation, ghost);
				
                if(node_victim == NULL){
					printf("\n[ERR0] SCMBP_get_victim returned NULL");
					exit(1);
				}
				if(node_victim->node->list != NULL){
					printf("\n[ERR0] SCMBP_get_victim returned a node with a list");
					exit(1);
				}
		    }else{
                printf("\n[ERROR] Buffer is full, but the sum of the sizes does not reflect this");
                exit(1);
            }

			struct Page * victim = (struct Page *) node_victim->page; /* Get the LRU Page */
			debug("\n ---- REPLACEMENT victim: %c[%d-%d]", victim->dirty_flag, victim->file_id, victim->block_id);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /* Read new data from storage media */
			
			set_dirty(page, operation);
			SCMBP_insert_new(node_victim, page->dirty_flag);

		}

	}
	set_dirty(page, operation);
	return page;
}

void SCMBP_insert_new(struct SCMBPNode * new_node, char operation){
	if (operation == READ_REQUEST) {
        SCMBP_insert_MRU(RECENT_CLEAN, new_node);
		P = MIN(P + Miss_Enlarger_C, (float) BUFFER_SIZE);
	} else if (operation == WRITE_REQUEST) {
		//SCMBP_insert_MRU(RECENT_DIRTY, new_node);
		SCMBP_insert_cluster(new_node);
		P = MAX(P - Miss_Enlarger_D, 0.0);
	}else{
		printf("\n\n[ERR0] invalid operation: %c",operation);
	}
}


struct SCMBPNode * get_victim(char operation, struct SCMBPNode * ghost ){
	
	struct SCMBPNode * node_victim = NULL;

	//Scan resistance
	if (operation == READ_REQUEST && RECENT_CLEAN->size == BUFFER_SIZE / 2) {
		node_victim = (struct SCMBPNode*) RECENT_CLEAN->tail->content;
		list_remove(RECENT_CLEAN, node_victim->node);
		SCMBP_destroy_ghost(ghost);
		return node_victim;

	} else if (operation == WRITE_REQUEST && RECENT_DIRTY_size() == BUFFER_SIZE / 2) {
	
		node_victim = get_cluster_victim();

		SCMBP_destroy_ghost(ghost);

		return node_victim;
		// -----------------
	} else
		//Cache starvation
		if (( (C() >= BUFFER_SIZE) || ( (RECENT_CLEAN->size + FREQUENT_CLEAN->size) >= BUFFER_SIZE) ) && ( (RECENT_CLEAN->size + FREQUENT_CLEAN->size) > 0 )) {
			return removePageC(ghost);

		} else if (D() >= 2 * BUFFER_SIZE) {
			return removePageD(ghost);
		} else {

			//normal case with "P"
			if ( ( (RECENT_CLEAN->size + FREQUENT_CLEAN->size) > (int) P) || ( (RECENT_DIRTY_size() + FREQUENT_DIRTY->size) == 0 )) {
				return removePageC(ghost);
			} else {
				return removePageD(ghost);
			}

		}

		if(node_victim == NULL){
			printf("\n[ERR0] get_victim returned NULL");
			exit(1);
		}

	return NULL;
}


struct SCMBPNode * removePageC(struct SCMBPNode * ghost){
	struct SCMBPNode * node_victim = NULL;

	if (RECENT_CLEAN->size > (int) PC || FREQUENT_CLEAN->size == 0) {
	
		ghost = remove_ghost_lru(GHOST_RECENT_CLEAN, ghost);
		node_victim = (struct SCMBPNode *) RECENT_CLEAN->tail->content;
		list_remove(RECENT_CLEAN, node_victim->node);
		SCMBP_insert_ghost(GHOST_RECENT_CLEAN, ghost, node_victim);
		
	} else {

		ghost = remove_ghost_lru(GHOST_FREQUENT_CLEAN, ghost);
		node_victim = (struct SCMBPNode *) FREQUENT_CLEAN->tail->content;
		list_remove(FREQUENT_CLEAN, node_victim->node);
		SCMBP_insert_ghost(GHOST_FREQUENT_CLEAN, ghost, node_victim);

	}
	return node_victim;
}


struct SCMBPNode * removePageD(struct SCMBPNode * ghost){
	struct SCMBPNode * node_victim = NULL;

	if (RECENT_DIRTY_size() > (int) PD || FREQUENT_DIRTY->size == 0) {
	
		ghost = remove_ghost_lru(GHOST_RECENT_DIRTY, ghost);
		node_victim = get_cluster_victim();

		SCMBP_insert_ghost(GHOST_RECENT_DIRTY, ghost, node_victim);

	} else {
		ghost = remove_ghost_lru(GHOST_FREQUENT_DIRTY, ghost);
		node_victim = (struct SCMBPNode *) FREQUENT_DIRTY->tail->content;
		list_remove(FREQUENT_DIRTY, node_victim->node);
		SCMBP_insert_ghost(GHOST_FREQUENT_DIRTY, ghost, node_victim);
	}
	return node_victim;
}





struct SCMBPNode * SCMBP_create_node(struct Page * page){
    struct SCMBPNode * scmbp_node = (struct SCMBPNode *) malloc (sizeof(struct SCMBPNode));
    struct Node * new_node = list_create_node(scmbp_node);

    scmbp_node->node = new_node;
    scmbp_node->page = page;
	scmbp_node->cluster = NULL;

    scmbp_node->ghost_bit = 0;

    scmbp_node->g_file_id = -1;
    scmbp_node->g_block_id = -1;

    if(page != NULL){
        page->extended_attributes = scmbp_node;
    }
    return scmbp_node;
}

void SCMBP_insert_ghost(struct List * list, struct SCMBPNode * ghost, struct SCMBPNode * node_victim){
	if (ghost->node->list != NULL){
		printf("\n[ERR0] ghost->node->list != NULL");
		exit(1);
	}
	ghost->g_file_id = node_victim->page->file_id;
    ghost->g_block_id = node_victim->page->block_id;
	SCMBP_insert_MRU(list, ghost);
}


void SCMBP_insert_MRU(struct List * list, struct SCMBPNode * node){
    list_insert_node_head(list ,node->node);
}

void SCMBP_insert_cluster(struct SCMBPNode * scmbp_node){
    struct Cluster * cluster = SCMBP_target_cluster(scmbp_node->page);
    
    if(cluster == NULL){
        cluster = SCMBP_create_cluster( SCMBP_cluster_id(scmbp_node->page) );
        //The cluster timestamp is the value of globaltime at the time of its creation.
        cluster->timestamp = GC;
        list_insert_node_head(RECENT_DIRTY, cluster->node);
    }

    /*
      When adding demoted pages to a existing cluster, 
      the cluster timestamp is not updated.
    */
    scmbp_node->cluster = cluster;
    list_insert_node_head(cluster->list, scmbp_node->node);

    GC++; // Each time a dirty page is evicted from the working region, globaltime is incremented by 1.
   
    //priority queue is adjusted.
    scmbp_node->cluster->priority = SCMBP_priority_function(cluster);
    //printf("  Priority: %d", scmbp_node->cluster->priority);
	if (cluster->list->size == 0){
		printf("\n[ERR0] when insert new node cluster->list->size == 0");
		exit(1);
	}
}

double SCMBP_priority_function(struct Cluster * cluster){
    if(cluster->list->head == NULL){
        printf("\n[ERR0] SCMBP: cluster->list->head == NULL");
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
        struct SCMBPNode * scmbp_node = (struct SCMBPNode *) node->content;
        int page_id = SCMBP_page_id(scmbp_node->page);
        
        struct SCMBPNode * prev_scmbp_node = (struct SCMBPNode *) node->prev->content;
        int prev_page_id = SCMBP_page_id(prev_scmbp_node->page);

        IPD = IPD + ABS( (page_id - prev_page_id) );

        node = node->next;
    }

    if(node == NULL){
        IPD = 1;
        return 1; //// ----------------> return 1??????
    }

    int n_2 = cluster->list->size * cluster->list->size; 
    double value = SAFE_DIVISION( (double) IPD,  n_2 * (GC - cluster->timestamp) );

    debug("---- %d / %d * (%d - %d) = %f", IPD, n_2, GC, cluster->timestamp,value);
    return value;

}

struct SCMBPNode * get_cluster_victim(){
	
    /*
    Once a victim is selected from a cluster, its priority is set to 0 and will not be updated
    anymore, so that the next victims will still be evicted from this victim cluster,
    resulting in strong spatial locality of page evictions.
    */
    
    struct Node * node = RECENT_DIRTY->tail;
    
    struct Cluster * cluster = NULL;
    struct Cluster * cluster_victim = NULL;

    double min = -1;

    while(node != NULL){
        cluster = (struct Cluster *) node->content;
        
        if(cluster->list->size == 0){
			buffer_print_policy();
            printf("\n[ERR0] SCMBP: cluster->list->size = 0, id: %d operation: %d", cluster->id, operations);
            exit(1);
        }
        if(cluster->priority < 0){
            printf("\n[ERR0] SCMBP: cluster->priority < 0");
            exit(1);
        }

        if(cluster->priority < min || min == -1){
            min = cluster->priority;
            cluster_victim = cluster;
        }

        node = node->prev;
    }

    if(cluster_victim == NULL){
        printf("\n[ERR0] SCMBP: cluster_victim == NULL");
        exit(1);
    }

    cluster_victim->priority = 0; // The cluster is marked
    
    struct Node * n_v = cluster_victim->list->tail;
    struct SCMBPNode * victim = (struct SCMBPNode*) n_v->content;


    if(victim == NULL){
        printf("\n[ERR0] victim is NULL");
        exit(1);
    }

   // printf("\n ---- VICTIM: %c[%d-%d]", victim->page->dirty_flag, victim->page->file_id, victim->page->block_id);
    
    SCMBP_remove_from_cluster(victim);

    if(cluster_victim->list->size == 0){
        list_remove(RECENT_DIRTY, cluster_victim->node);
        SCMBP_destroy_cluster(cluster_victim);
    }
    
    return victim;
}



struct Cluster * SCMBP_create_cluster(int id){
    struct Cluster * cluster = (struct Cluster *) malloc(sizeof(struct Cluster));
    cluster->list = list_create(NULL,NULL);
    cluster->id = id;
    cluster->priority = -1;
    cluster->timestamp = -1; 
    cluster->node = list_create_node(cluster);
    return cluster;
}


struct Cluster* SCMBP_target_cluster(struct Page * x){
   
	int cluster_id = SCMBP_cluster_id(x);

	struct Node * n = RECENT_DIRTY->head;
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

struct SCMBPNode * SCMBP_remove_from_cluster(struct SCMBPNode * scmbp_node){
    struct Cluster * cluster = scmbp_node->cluster;
    if(cluster == NULL){
        printf("\n[ERR0] SCMBP: node is not in any cluster");
        exit(1);
    }
    
    list_remove(cluster->list, scmbp_node->node);   
    scmbp_node->cluster = NULL;

    return scmbp_node;
}




void SCMBP_destroy_cluster(struct Cluster * cluster){
   
    list_free_node(cluster->list, cluster->node);
    
    if (cluster->list != NULL && cluster->list->size > 0){
        printf("\n[ERR0] SCMBP: cluster list is not empty");
        exit(1);
    }

    list_free(cluster->list);    
    
    free(cluster);
}

int SCMBP_page_id(struct Page * page){
    //Use Cantor pairing function to get the cluster id
    //(a + b) * (a + b + 1) / 2 + a; where a, b >= 0
    //int page_id = (page->file_id + page->block_id) * (page->file_id + page->block_id + 1) / 2 + page->block_id;
    return (int) page->block_id;
}

int SCMBP_cluster_id(struct Page * page){

    return SCMBP_page_id(page) / MAX_CLUSTER_SIZE;
}


void SCMBP_destroy_ghost(struct SCMBPNode * ghost_node){
	if(ghost_node != NULL){
		struct List * temp = ghost_node->node->list;
		if (temp == NULL){
			printf("\n[ERR0] ghost_node->node->list == NULL");
			exit(1);
		}

		list_remove(temp, ghost_node->node);

		if(ghost_node->page != NULL){
			printf("\n[ERR0] Page found in Ghost Buffer");
			exit(1);
		}

		if(ghost_node->cluster != NULL){
			printf("\n[ERR0] Cluster found in Ghost Buffer");
			exit(1);
		}

		list_free_node(temp, ghost_node->node);
		ghost_node->node = NULL;
		ghost_node->page = NULL;
		free(ghost_node);
	}
}

struct SCMBPNode * remove_ghost_lru(struct List * list, struct SCMBPNode * ghost){
	
	if(ghost != NULL){
		list_remove(ghost->node->list, ghost->node);
		return ghost;
	}

	if (Do() + Co() == BUFFER_SIZE) {
		struct Node * removed = list_remove_tail(list);

		if (removed == NULL) {
			removed = select_ghost_victim();
			list_remove(removed->list, removed);
		}
		return (struct SCMBPNode *) removed->content;
	}

	ghost = SCMBP_create_node(NULL);
	ghost->ghost_bit = TRUE;
	return ghost;
}

void SCMBP_ajust_ghost_values(struct Node * x, char operation){
	if (operation == READ_REQUEST) {
		P = MIN(P + Ghost_Hit_Enlarger_C, (float) BUFFER_SIZE);
	} else {
		P = MAX(P - Ghost_Hit_Enlarger_D, 0.0);
	}
	if (x->list == GHOST_RECENT_CLEAN) {

		PC = MIN(PC + Ghost_Hit_Enlarger_RecentClean, (float) BUFFER_SIZE);

	} else

	if (x->list == GHOST_FREQUENT_CLEAN) {

		PC = MAX(PC - Ghost_Hit_Enlarger_FrequentClean, 0.0);

	} else

	if (x->list == GHOST_RECENT_DIRTY) {

		PD = MIN(PD + Ghost_Hit_Enlarger_RecentDirty, (float) BUFFER_SIZE);

	} else

	if (x->list == GHOST_FREQUENT_DIRTY) {

		PD = MAX(PD - Ghost_Hit_Enlarger_FrequentDirty, 0.0);

	} else {
		printf("\n[ERR0] Node not in Ghost list");
		exit(1);
	}
}

struct SCMBPNode * find_ghost(int file_id, int block_id){
	struct SCMBPNode * ghost = NULL;
	ghost = find_ghost_list(GHOST_RECENT_CLEAN, file_id, block_id);
	if(ghost != NULL)return ghost;

	ghost = find_ghost_list(GHOST_FREQUENT_CLEAN, file_id, block_id);
	if(ghost != NULL)return ghost;

	ghost = find_ghost_list(GHOST_RECENT_DIRTY, file_id, block_id);
	if(ghost != NULL)return ghost;

	ghost = find_ghost_list(GHOST_FREQUENT_DIRTY, file_id, block_id);
	return NULL;
}

struct SCMBPNode * find_ghost_list(struct List * list, int file_id, int block_id){
    struct Node * x = list->head;
        while(x != NULL){
            struct SCMBPNode * ghost = (struct SCMBPNode *) x->content;
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

struct Node * select_ghost_victim() {
	struct Node * x = NULL;
	x = GHOST_RECENT_CLEAN->head;
	if (x != NULL) {
		
		return x;
	}
	x = GHOST_FREQUENT_CLEAN->head;
	if (x != NULL) {
		
		return x;
	}
	x = GHOST_RECENT_DIRTY->head;
	if (x != NULL) {
		
		return x;
	}
	x = GHOST_FREQUENT_DIRTY->head;
	if (x != NULL) {
		
		return x;
	}

	return x;
}



void buffer_print_policy(){
	
//GHOST_RECENT_CLEAN, RECENT_CLEAN, FREQUENT_CLEAN, GHOST_FREQUENT_CLEAN
//GHOST_RECENT_DIRTY, RECENT_DIRTY, FREQUENT_DIRTY, GHOST_FREQUENT_DIRTY
	struct Node * x = GHOST_RECENT_CLEAN->tail;
    printf("\n<- ");
    while(x!=NULL){
		struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        printf(" %c[%d-%ld]", 'G', nodeB->g_file_id, nodeB->g_block_id);
        x = x->prev;
    }
	x = RECENT_CLEAN->tail;
    printf(" <-");
    while(x!=NULL){
        struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        struct Page * page = nodeB->page;
        printf(" %c[%d-%ld]", page->dirty_flag, page->file_id, page->block_id);
        x = x->prev;
    }
	x = FREQUENT_CLEAN->head;
    printf("<- ! -> ");
    while(x!=NULL){
        struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        struct Page * page = nodeB->page;
        printf(" %c[%d-%ld]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
	x = GHOST_FREQUENT_CLEAN->head;
    printf(" ->");
    while(x!=NULL){
        struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        printf(" %c[%d-%ld]", 'G', nodeB->g_file_id, nodeB->g_block_id);
        x = x->next;
    }
	printf(" ->");
	//------------------------
	x = GHOST_RECENT_DIRTY->tail;
    printf("\n<- ");
    while(x!=NULL){
        struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        printf(" %c[%d-%ld]", 'G', nodeB->g_file_id, nodeB->g_block_id);
        x = x->prev;
    }
	x = RECENT_DIRTY->tail;
    printf("<- ");
    if(RECENT_DIRTY->size > 0){
        
        while(x!=NULL){
            struct Cluster * cluster = (struct Cluster *) x->content;
            struct Node * n = cluster->list->head;
            printf(" P:%.3f;id:%d;T:%d(",cluster->priority, cluster->id, cluster->timestamp);
            //printf(" size:%d;", cluster->list->size);
            while(n!=NULL){
                struct SCMBPNode * scmbp_node = (struct SCMBPNode *) n->content;
                struct Page * page = scmbp_node->page;
                printf(" %c[%d-%d]", page->dirty_flag, page->file_id, page->block_id);
                n = n->next;
            }
        	printf(")");
        	x = x->prev;
        }
    }

	x = FREQUENT_DIRTY->head;
    printf("<- ! -> ");
    while(x!=NULL){
        struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        struct Page * page = nodeB->page;
        printf(" %c[%d-%ld]", page->dirty_flag, page->file_id, page->block_id);
        x = x->next;
    }
	x = GHOST_FREQUENT_DIRTY->head;
    printf(" ->");
    while(x!=NULL){
      	struct SCMBPNode * nodeB = (struct SCMBPNode *) x->content;
        printf(" %c[%d-%ld]", 'G', nodeB->g_file_id, nodeB->g_block_id);
        x = x->next;
    }
	printf(" ->");
}

#endif
