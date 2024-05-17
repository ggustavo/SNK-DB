/*

 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"

/*
*                                                -- NEW Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct MLNode   |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

struct List * hot_clean;
struct List * hot_dirty;
struct List * cold_clean;
struct List * cold_dirty;

struct List * ghost_list;
struct List * historic_list_in;
struct List * historic_list_out;

// static values
double ALFA;
int historic_size_in;
int historic_size_out;
int min_cold_clean_size;
int min_cold_dirty_size;
int min_hot_clean_size;
int min_hot_dirty_size;

// dynamic values
int GC;
double write_intensity = 0.5;
double read_intensity = 0.5;
int count_ghosts = 0;

int desirable_cold_clean_size;
int desirable_cold_dirty_size;
int desirable_hot_clean_size;
int desirable_hot_dirty_size;


struct MLNode{
    struct Page * page;
    struct Node * node;
    double references; // Frequency
    int first; // First access time
    int last; // Last access time
    double ES; // Forecast
    double ERROR;
};

struct HistoricNodeIn{
    int file_id;   
	long block_id;  
    char dirty_flag;
};



struct HistoricNodeOut{
    int file_id;   
	long block_id;  
    char dirty_flag;

    double references; // Frequency
    int first; // First access time
    int last; // Last access time
    double ES; // Forecast
    double ERROR;
    int ghost_GC; // time the page entered the ghost list
    struct Node * node;
};


struct MLNode * NEW_create_node(struct Page * page);
void NEW_insert(struct List * list, struct MLNode * node);
struct MLNode * get_MLNode(struct Node * node);
double NEW_PC(struct MLNode * lru_node);
struct MLNode * get_victim();
void NEW_print_PC(struct Node * x);
void ES();
void add_historic_in(struct Page * page);
void add_historic_out(struct MLNode * ml_node, struct HistoricNodeOut * ghost);

void buffer_print_policy_historic_in();
void buffer_print_policy_historic_out();
double calc_score(struct MLNode * node);
void analyze();
struct HistoricNodeOut * find_ghost(int file_id, long block_id);

struct HistoricNodeOut * aux = NULL;

/*
 * This function is called after initializing the buffer and before page requests.
 * 
 * Initialize the structures used in the page replacement policy here.
 */
void buffer_policy_start(){
    aux = (struct HistoricNodeOut *) malloc(sizeof(struct HistoricNodeOut));
    ALFA = 0.5;
    GC = 0;
    historic_size_in = 1000;
    historic_size_out = BUFFER_SIZE;

    min_cold_clean_size = MAX(1, (int) (BUFFER_SIZE * 0.10) );
    min_cold_dirty_size = MAX(1, (int) (BUFFER_SIZE * 0.10) );
    min_hot_clean_size = MAX(1, (int) (BUFFER_SIZE * 0.10) );
    min_hot_dirty_size = MAX(1, (int) (BUFFER_SIZE * 0.10) );

    desirable_cold_clean_size = min_cold_clean_size;
    desirable_cold_dirty_size = min_cold_dirty_size;
    desirable_hot_clean_size = min_hot_clean_size;
    desirable_hot_dirty_size = min_hot_dirty_size;


    printf("\nBuffer Replacement Policy: %s \nALFA:%f Historic Size:%d", __FILE__, ALFA, historic_size_in);
    printf("\n---------------------------------------------------------------------------------------------------");
    hot_clean  = list_create(NULL,NULL);
    hot_dirty  = list_create(NULL,NULL);
    cold_clean = list_create(NULL,NULL);
    cold_dirty = list_create(NULL,NULL);
    ghost_list = list_create(NULL,NULL);
    historic_list_in = list_create(NULL,NULL);
    historic_list_out = list_create(NULL,NULL);

    for (int i = 0; i < historic_size_in; i++){
        struct HistoricNodeIn * h_node = (struct HistoricNodeIn *) malloc(sizeof(struct HistoricNodeIn));
        h_node->file_id = -1;
        h_node->block_id = -1;
        h_node->dirty_flag = '-';
        struct Node * node = list_create_node(h_node);
        list_insert_node_head(historic_list_in, node);
    }

    for(int i = 0; i < historic_size_out; i++){
        struct HistoricNodeOut * h_node = (struct HistoricNodeOut *) malloc(sizeof(struct HistoricNodeOut));
        h_node->file_id = -1;
        h_node->block_id = -1;
        h_node->dirty_flag = '-';
        struct Node * node = list_create_node(h_node);
        list_insert_node_head(historic_list_out, node);
        h_node->node = node;
    }
}

struct Page * buffer_request_page(int file_id, long block_id, char operation){

	// It is mandatory to call this two functions (buffer_find_page, buffer_computes_request_statistics)
	//--------------------------------------------------------
	struct Page * page = buffer_find_page(file_id, block_id);
	buffer_computes_request_statistics(page, operation);
	//--------------------------------------------------------

    GC++;
    
	if(page != NULL){ /* HIT - Update references */

		struct MLNode * ml_node = (struct MLNode *) page->extended_attributes;
        ml_node->references =  ml_node->references  + 1;
        ES(ml_node);
        ml_node->last = GC;

        list_remove(ml_node->node->list,ml_node->node); // Removes the node from the current list

        if (operation == READ_REQUEST && page->dirty_flag == PAGE_CLEAN){
            NEW_insert(hot_clean, ml_node);
        }else{
            NEW_insert(hot_dirty, ml_node);
        }
        
	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */
        
		if (buffer_is_full() == FALSE) {

			page = buffer_get_free_page();
			struct MLNode * new_node = NEW_create_node(page);
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            
            if (operation == READ_REQUEST){
                NEW_insert(cold_clean, new_node);
            }else{
                NEW_insert(cold_dirty, new_node);
            }

		} else { /*  Need a replacement */

            struct HistoricNodeOut * ghost = find_ghost(file_id, block_id);
            
            if (ghost != NULL){
                aux->block_id = ghost->block_id;
                aux->file_id = ghost->file_id;
                aux->dirty_flag = ghost->dirty_flag;
                aux->references = ghost->references;
                aux->first = ghost->first;
                aux->last = ghost->last;
                aux->ES = ghost->ES;
                aux->ERROR = ghost->ERROR;
                aux->ghost_GC = ghost->ghost_GC;
            }

            analyze();

            struct MLNode * node_victim = get_victim();
            list_remove(node_victim->node->list, node_victim->node);
            
            add_historic_out(node_victim, ghost);

            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->references);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */


            if(ghost != NULL){

                count_ghosts ++;
              
                /*
                int distance = ABS( ( (aux->ghost_GC - aux->first) - (GC - aux->first) ) );
                printf("\n[%c] D(%d) Lifetime: %d/%d (F:%d-L:%d-G:%d) F:%f ES:%f ERR:%f ", 
                aux->dirty_flag,
                distance,
                aux->ghost_GC - aux->first, 
                GC - aux->first,
                aux->first, 
                aux->last, 
                aux->ghost_GC,
                aux->references,
                aux->ES, 
                aux->ERROR
                );
                */

                if(aux->references>1){
                    //printf("\nFREQUENT PAGE");
                }	
                
                node_victim->references = aux->references + 1;
                node_victim->first = aux->first;
                node_victim->ES = aux->ES;
                node_victim->ERROR = aux->ERROR; 
                ES(node_victim);
                node_victim->last = GC;
                
                if (operation == READ_REQUEST){
                    NEW_insert(hot_clean, node_victim);
                }else{
                    NEW_insert(hot_dirty, node_victim);
                } 

            }else{
                node_victim->references = 1.0;
                node_victim->last = GC;
                node_victim->first = GC;
                node_victim->ES = 0;
                node_victim->ERROR = 0;   

                if (operation == READ_REQUEST){
                    NEW_insert(cold_clean, node_victim);
                }else{
                    NEW_insert(cold_dirty, node_victim);
                } 
            }

            
		}

	}
    add_historic_in(page);
	set_dirty(page, operation);
	return page;
}

struct MLNode * get_MLNode(struct Node * node){
    if (node == NULL) return NULL;
    return (struct MLNode *) node->content;
}




// Exponential smoothing
// E(t) = E(t-1) + alfa * ( d(t) - E(t-1) )
void ES(struct MLNode * node){
    // (GC - node->last) is the distance between the last time the page was used and now
    double distance = (double) (GC - node->last);
    if (node->ES == -1){ // second reference now
        node->ES = distance;
        node->ERROR = 0;
    }else{
        node->ERROR = ABS( (distance - node->ES) );
    }
    
    node->ES = node->ES + ALFA * ( distance - node->ES);
    
    //if(node->page->block_id == 1){
    //    printf("\n OP: %d // D: %f ES: %f ERROR:%f", operations, distance, node->ES, node->ERROR);
    //}
}

double calc_score(struct MLNode * node){
    double result = 0.0;
    //double ES_score = SAFE_DIVISION ( (node->ES + node->ERROR) , node->ERROR);

    double ES_score = node->ES + node->ERROR;

    double R2 = (node->references - ES_score ) / node->references;
    //double R2 = ES_score / node->references;  

    //printf("\n WI: %f, %c[%d-%ld] F:%f ES:%f R2:%f", write_intensity,
    //node->page->dirty_flag, node->page->file_id, node->page->block_id, node->references, ES_score, R2);
    
    result = R2;
    
    if(node->page->dirty_flag == PAGE_DIRTY){
        result = (result * write_intensity) + write_intensity;
    }else{
        result = (result * read_intensity) + read_intensity;
    }
    
    return result;
}

struct MLNode * get_victim(){
    
    struct MLNode * P_CC = get_MLNode(cold_clean->tail);
    struct MLNode * P_CD = get_MLNode(cold_dirty->tail);
    struct MLNode * P_HC = get_MLNode(hot_clean->tail);
    struct MLNode * P_HD = get_MLNode(hot_dirty->tail);

    desirable_cold_clean_size = min_cold_clean_size * read_intensity;
    desirable_cold_dirty_size = min_cold_dirty_size * write_intensity;
    desirable_hot_clean_size = min_hot_clean_size * read_intensity;
    desirable_hot_dirty_size = min_hot_dirty_size * write_intensity;


    // Normalize all the values
    struct MLNode * nodes[4] = {P_CC, P_CD, P_HC, P_HD};

    if(cold_clean->size <= desirable_cold_clean_size){
        nodes[0] = NULL;
    }

    if(cold_dirty->size <= desirable_cold_dirty_size){
        nodes[1] = NULL;
    }

    if(hot_clean->size <= desirable_hot_clean_size ){
        nodes[2] = NULL;
    }

    if(hot_dirty->size <= desirable_hot_dirty_size ){
        nodes[3] = NULL;
    }

    struct MLNode * victim = NULL;
    double note_victim = 0;
    double note_note = 0;
    for (size_t i = 0; i < 4; i++){
        if (nodes[i] != NULL){
            if (victim == NULL){
                victim = nodes[i];
                note_victim = calc_score(victim);                
            }else{
                note_note = calc_score(nodes[i]);
            
                if (note_note > note_victim){
                    victim = nodes[i];
                }
            }
        }
    }


    /*
    printf("\n-------------------------------------------------------");
    if(P_CC!=NULL) printf("\nCC: %f cold_clean:%d", calc_score(P_CC),cold_clean->size);
    if(P_CD!=NULL) printf("\nCD: %f cold_dirty:%d", calc_score(P_CD),cold_dirty->size);
    if(P_HC!=NULL) printf("\nHC: %f hot_clean:%d", calc_score(P_HC),hot_clean->size);
    if(P_HD!=NULL) printf("\nHD: %f hot_dirty:%d", calc_score(P_HD),hot_dirty->size);
    printf("\n-- VIC:%f --", calc_score(victim));
    */

    if (victim == NULL){
        printf("\n-------------------------------------------------------");
        if(P_CC!=NULL) printf("\nCC: %f cold_clean:%d/%d", calc_score(P_CC),cold_clean->size, desirable_cold_clean_size);
        if(P_CD!=NULL) printf("\nCD: %f cold_dirty:%d/%d", calc_score(P_CD),cold_dirty->size, desirable_cold_dirty_size);
        if(P_HC!=NULL) printf("\nHC: %f hot_clean:%d/%d", calc_score(P_HC),hot_clean->size, desirable_hot_clean_size);
        if(P_HD!=NULL) printf("\nHD: %f hot_dirty:%d/%d", calc_score(P_HD),hot_dirty->size, desirable_hot_dirty_size);
        
        printf("\n[ERR0R]: No victim found OPP:%d", operations);
        exit(1);
    }
    return victim;
}


void analyze(){
    struct Node * x = historic_list_in->tail;
    int write_counter = 0;
    int read_counter = 0;
    int count = 0;

    long prev = 0;
    long distance = 0;
    long sum = 0;
    
    while(x != NULL){
        struct HistoricNodeIn * n = (struct HistoricNodeIn *) x->content;
        if(n->file_id == -1) break;
        
        // READ and WRITE intensity
        if(n->dirty_flag == PAGE_DIRTY){
            write_counter++;
        }else{
            read_counter++;
        }
        count++;
        x = x->prev;
    }
    write_intensity =  ( (write_counter * 100.0 ) / (double) count ) / 100.0;
    read_intensity =   ( (read_counter * 100.0 ) / (double) count ) / 100.0;
 

    //printf("\n W:%f R:%f", write_intensity, read_intensity);

}

void add_historic_in(struct Page * page){
    struct Node * free_n = list_remove_tail(historic_list_in);
    struct HistoricNodeIn * h_node = (struct HistoricNodeIn *) free_n->content;
    h_node->file_id = page->file_id;
    h_node->block_id = page->block_id;
    h_node->dirty_flag = page->dirty_flag;
    list_insert_node_head(historic_list_in, free_n);
}

void add_historic_out(struct MLNode * ml_node, struct HistoricNodeOut * ghost){
    struct Node * free_n = NULL;

    if(ghost == NULL){
        free_n = list_remove_tail(historic_list_out);
    }else{
        free_n = list_remove(historic_list_out, ghost->node);
    }

    struct HistoricNodeOut * h_node = (struct HistoricNodeOut *) free_n->content;
    
    h_node->file_id = ml_node->page->file_id;
    h_node->block_id = ml_node->page->block_id;
    h_node->dirty_flag = ml_node->page->dirty_flag;

    h_node->references = ml_node->references;
    h_node->first = ml_node->first;
    h_node->last = ml_node->last;
    h_node->ES = ml_node->ES;
    h_node->ERROR = ml_node->ERROR;
    h_node->ghost_GC = GC;

    // ml_node->first 
    list_insert_node_head(historic_list_out, free_n);
}


void NEW_insert(struct List * list, struct MLNode * ml_node){
    list_insert_node_head(list,ml_node->node);
}

struct MLNode * NEW_create_node(struct Page * page){
    struct MLNode * ml_node = (struct MLNode *) malloc (sizeof(struct MLNode));
    
    struct Node * new_node = list_create_node(ml_node);
    
    ml_node->references = 1.0;
    ml_node->node = new_node;
    ml_node->page = page;   
    ml_node->last = GC;
    ml_node->first = GC;
    ml_node->ES = 0;
    ml_node->ERROR = 0;

    page->extended_attributes = ml_node;
    return ml_node;
}

void buffer_print_policy(){
    struct Node * x = hot_clean->tail;
    printf("\n");
    while(x!=NULL){
        struct MLNode * n = (struct MLNode *) x->content;
        printf(" %c[%d-%ld]:%f",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->prev;
    }
    x = cold_clean->tail;
    printf(" <-");
    while(x!=NULL){
        struct MLNode * n = (struct MLNode *) x->content;
        printf(" %c[%d-%ld]:%f",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->prev;
    }
    x = cold_dirty->head;
    printf(" <- ... ->");
    while(x!=NULL){
        struct MLNode * n = (struct MLNode *) x->content;
        printf(" %c[%d-%ld]:%f",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->next;
    }
    x = hot_dirty->head;
    printf(" ->");
    while(x!=NULL){
        struct MLNode * n = (struct MLNode *) x->content;
        printf(" %c[%d-%ld]:%f",n->page->dirty_flag, n->page->file_id, n->page->block_id, n->references);
        x = x->next;
    }
    //buffer_print_policy_historic_in();
}

void buffer_print_policy_historic_in(){
    struct Node * x = historic_list_in->tail;
    printf("\nIN:");
    while(x!=NULL){
        struct HistoricNodeIn * n = (struct HistoricNodeIn *) x->content;
        printf(" %c[%d-%ld]",n->dirty_flag, n->file_id, n->block_id);
        x = x->prev;
    }
}


void buffer_print_policy_historic_out(){
    struct Node * x = historic_list_out->tail;
    printf("\nOUT:");
    while(x!=NULL){
        struct HistoricNodeOut * n = (struct HistoricNodeOut *) x->content;
        printf(" %c[%d-%ld]",n->dirty_flag, n->file_id, n->block_id);
        x = x->prev;
    }
}

 struct HistoricNodeOut * find_ghost(int file_id, long block_id){
    struct Node * x = historic_list_out->tail;

    while(x!=NULL){
        struct HistoricNodeOut * n = (struct HistoricNodeOut *) x->content;
        if(n->file_id == file_id && n->block_id == block_id){
            
            return n;
        }
        x = x->prev;
    }
    return NULL;
}

#endif
