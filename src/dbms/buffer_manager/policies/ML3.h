/*

 */
#ifndef POLICY_H_INCLUDED
#define POLICY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "../db_buffer.h"
#include "../../db_config.h"
#include "../../util/hash_table.h"


/*
*                                                -- NEW Structure --
* 
*                    void *extended_attributes                           struct Node * node
*    |-------------|-----------------------------> |------------------|-----------------------> |-------------|
*    | struct Page |                               |  struct MLNode   |                         | struct Node |
*    |-------------| <-----------------------------|------------------| <-----------------------|-------------|
*                         struct Page * page                                 void * content
*/

int count_evicted_cold_clean = 0;
int count_evicted_cold_dirty = 0;
int count_evicted_hot_clean = 0;
int count_evicted_hot_dirty = 0;

struct List * hot_clean;
struct List * hot_dirty;
struct List * cold_clean;
struct List * cold_dirty;

struct List * ghost_list;

struct Hash * ghost_hash;

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

double write_hit_intensity = 0;
double read_hit_intensity = 0;

double write_ghost_hit_intensity = 0;
double read_ghost_hit_intensity = 0;

double write_miss_intensity = 0;
double read_miss_intensity = 0;

double max_page_frequency_interval = 0;

int desirable_cold_clean_size;
int desirable_cold_dirty_size;
int desirable_hot_clean_size;
int desirable_hot_dirty_size;

double score_cold_clean;
double score_cold_dirty;
double score_hot_clean;
double score_hot_dirty;

double write_cost;
double read_cost;

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
    int is_miss;
    int is_hit;
    int is_ghost_hit;
    double references;
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

struct HistoricNodeIn ** historic_in;
struct List * historic_list_out;

struct MLNode * NEW_create_node(struct Page * page);
void NEW_insert(struct List * list, struct MLNode * node);
struct MLNode * get_MLNode(struct Node * node);
double NEW_PC(struct MLNode * lru_node);
struct MLNode * get_victim();
void NEW_print_PC(struct Node * x);
void ES();
void add_historic_in(struct Page * page, double references, int is_hit, int is_miss, int is_ghost_hit);
void add_historic_out(struct MLNode * ml_node, struct HistoricNodeOut * ghost);

void buffer_print_policy_historic_in();
void buffer_print_policy_historic_out();
double calc_score(struct MLNode * node);
void * analyze(void * arg);
double reward(double x);
double punish(double x);

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
    historic_size_in = BUFFER_SIZE * 0.1;
    historic_size_out = BUFFER_SIZE;

    min_cold_clean_size = MAX(1, (int) (BUFFER_SIZE * 0.10) );
    min_cold_dirty_size = MAX(1, (int) (BUFFER_SIZE * 0.10) );
    min_hot_clean_size  = MAX(1, (int) (BUFFER_SIZE * 0.10) );
    min_hot_dirty_size  = MAX(1, (int) (BUFFER_SIZE * 0.10) );

    desirable_cold_clean_size = min_cold_clean_size;
    desirable_cold_dirty_size = min_cold_dirty_size;
    desirable_hot_clean_size = min_hot_clean_size;
    desirable_hot_dirty_size = min_hot_dirty_size;

    write_cost = 0.8;
    read_cost = 0.2;

    score_cold_clean = 50.0;
    score_cold_dirty = 50.0;
    score_hot_clean = 50.0;
    score_hot_dirty = 50.0;

    printf("\nBuffer Replacement Policy: %s \nALFA:%f Historic Size:%d", __FILE__, ALFA, historic_size_in);
    printf("\n---------------------------------------------------------------------------------------------------");
    hot_clean  = list_create(NULL,NULL);
    hot_dirty  = list_create(NULL,NULL);
    cold_clean = list_create(NULL,NULL);
    cold_dirty = list_create(NULL,NULL);
    ghost_list = list_create(NULL,NULL);
    historic_in = (struct HistoricNodeIn **) malloc(historic_size_in * sizeof(struct HistoricNodeIn*));
    historic_list_out = list_create(NULL,NULL);
    
    ghost_hash = hash_table_create(6929239);

    for (int i = 0; i < historic_size_in; i++){
        struct HistoricNodeIn * h_node = (struct HistoricNodeIn *) malloc(sizeof(struct HistoricNodeIn));
        h_node->file_id = -1;
        h_node->block_id = -1;
        h_node->dirty_flag = '-';
        historic_in[i] = h_node;
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
    int is_ghost_hit = 0;
    int is_hit = 0;
    int is_miss = 0;
    double references = 0.0;

	if(page != NULL){ /* HIT - Update references */
        is_hit = 1;

		struct MLNode * ml_node = (struct MLNode *) page->extended_attributes;
        ml_node->references =  ml_node->references  + 1;
        ES(ml_node);
        ml_node->last = GC;

        if (ml_node->node->list == cold_clean){
            score_cold_clean = reward(score_cold_clean + 1 + read_cost);
        }
        else if (ml_node->node->list == cold_dirty){
            score_cold_dirty = reward(score_cold_dirty + 1 + write_cost);
        }
        else if (ml_node->node->list == hot_clean){
            score_hot_clean = reward(score_hot_clean + 1 + read_cost);
        }
        else if (ml_node->node->list == hot_dirty){
            score_hot_dirty = reward(score_hot_dirty + 1 + write_cost);
        }

        list_remove(ml_node->node->list,ml_node->node); // Removes the node from the current list

        if (operation == READ_REQUEST && page->dirty_flag == PAGE_CLEAN){
            NEW_insert(hot_clean, ml_node);    
        }else{
            NEW_insert(hot_dirty, ml_node);
        }

        references = ml_node->references;
        
	} else { /* MISS - page is not in Buffer (struct Page * page == NULL) */
        
		if (buffer_is_full() == FALSE) {
            is_miss = 1;

			page = buffer_get_free_page();
			struct MLNode * new_node = NEW_create_node(page);
        
			buffer_load_page(file_id, block_id, page); /* Read the data from storage media */
            
            if (operation == READ_REQUEST){
                NEW_insert(cold_clean, new_node);
            }else{
                NEW_insert(cold_dirty, new_node);
            }
            references = 1.0; 

		} else { /*  Need a replacement */
            is_miss = 1;

            struct HistoricNodeOut * ghost = find_ghost(file_id, block_id);

            if (ghost != NULL){
                is_ghost_hit = 1;

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


            /*
            if(  GC > 0 &&  ( GC % 100 == 0 ) ){
                analyze(NULL);  
            }
            */
            

            struct MLNode * node_victim = get_victim();
            list_remove(node_victim->node->list, node_victim->node);
            
            add_historic_out(node_victim, ghost);

            struct Page * victim = node_victim->page;

            debug("\n ---- REPLACEMENT victim: %c[%d-%d]-%d", victim->dirty_flag, victim->file_id, victim->block_id,node_victim->references);

			buffer_flush_page(victim); /* Flush the data to the secondary storage media if is dirty */
			page = buffer_reset_page(victim); /* To avoid malloc a new page we reuse the victim page */
			buffer_load_page(file_id, block_id, page); /*  Read new data from storage media */

        
            
            if(ghost != NULL){

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

                if(aux->references > 1){
                    //printf("\nFREQUENT PAGE");
                    if(aux->dirty_flag == PAGE_CLEAN){
                        score_hot_clean = punish(score_hot_clean - (1 + read_cost) );
                    }else{
                        score_hot_dirty = punish(score_hot_dirty - (1 + write_cost) );
                    }
                }else{
                  
                    if(aux->dirty_flag == PAGE_CLEAN){
                        score_cold_clean = punish(score_cold_clean - (1 + read_cost) );
                    }else{
                        score_cold_dirty = punish(score_cold_dirty - (1 + write_cost) );
                    }
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
                    score_cold_clean = punish(score_cold_clean - 1 - read_cost);
                }else{
                    NEW_insert(cold_dirty, node_victim);
                    score_cold_dirty = punish(score_cold_dirty - 1 - write_cost);
                } 
            }

            references = node_victim->references;
            
		}

	}
	set_dirty(page, operation);

    add_historic_in(page, references, is_hit, is_miss, is_ghost_hit);
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
    if (node->ES == 0){ // second reference now
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
    if(node == NULL){
        printf("\n[ERR0] node is NULL");
        exit(1);
    }

    //FATORES A CONSIDERAR:
    // ES
    // ERROR
    // REFERENCES
    // TEMPO EM BUFFER
    // CUSTO DA OPERACAO
    // INTENSIDADE DA OPERACAO

    
    double result = 0.0;
    
    //double error_calc =  SAFE_DIVISION(node->references, node->ERROR);
    //ouble ES_calc = SAFE_DIVISION(node->references, node->ES);
    //double error_calc =  SAFE_DIVISION(node->ERROR, node->references);
    //double ES_calc = SAFE_DIVISION(node->ES,node->references);

    result = SAFE_DIVISION( (node->ES + node->ERROR) , (node->references) );

    if(node->page->dirty_flag == PAGE_DIRTY){
        //result = result - (write_intensity + write_hit_intensity + (write_ghost_hit_intensity * write_ghost_hit_intensity));
        //result = result + write_miss_intensity;
        //result = result - (result * write_cost);
        result = (result * write_intensity) + (1 - write_cost); // COLOQUEI UM + 
    }else{
        //result = result - (read_intensity + read_hit_intensity + (read_ghost_hit_intensity * read_ghost_hit_intensity));
       // result = result + read_miss_intensity;
       // result = result - (result * read_cost);
        result = (result * read_intensity) + (1 - read_cost); // COLOQUEI UM + 
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
    
    
    if(read_miss_intensity > 0 
    && (cold_clean->size > (BUFFER_SIZE  * (read_miss_intensity * read_cost) ) ) ){
        nodes[0] = P_CC;
        nodes[1] = NULL;
        nodes[2] = NULL;
        nodes[3] = NULL;
    }

    if(write_miss_intensity > 0 
    && (cold_dirty->size > (BUFFER_SIZE * ( write_miss_intensity * write_cost) ) ) ){
        nodes[0] = NULL;
        nodes[1] = P_CD;
        nodes[2] = NULL;
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
                    note_victim = note_note; /// SEE THAT PLS <<<
                }
            }
        }
    }


    /*
    printf("\n-------------------------------------------------------");
    if(P_CC!=NULL) printf("\nCC: %f \tcold_clean: %d \ts: %f", calc_score(P_CC),cold_clean->size, score_cold_clean);
    if(P_CD!=NULL) printf("\nCD: %f \tcold_dirty: %d \ts: %f", calc_score(P_CD),cold_dirty->size, score_cold_dirty);
    if(P_HC!=NULL) printf("\nHC: %f \thot_clean: %d  \ts: %f", calc_score(P_HC),hot_clean->size, score_hot_clean);
    if(P_HD!=NULL) printf("\nHD: %f \thot_dirty: %d  \ts: %f", calc_score(P_HD),hot_dirty->size, score_hot_dirty);
    printf("\n-- VICTIM: %f -- MAX_FREQUENCY: %f", calc_score(victim), max_page_frequency_interval);
    printf("\n-- READ: %f    \tWRITE: %f --", read_intensity, write_intensity);
    printf("\n-- R_HIT: %f   \tW_HIT: %f --", read_hit_intensity, write_hit_intensity);
    printf("\n-- R_MISS: %f  \tW_MISS: %f --", read_miss_intensity, write_miss_intensity);
    printf("\n-- R_GHOST: %f \tW_GHOST: %f --", read_ghost_hit_intensity, write_ghost_hit_intensity);
    */

    /*
    printf("\n-- evicted_cold_clean: %d", count_evicted_cold_clean);
    printf("\n-- evicted_cold_dirty: %d", count_evicted_cold_dirty);
    printf("\n-- evicted_hot_clean: %d", count_evicted_hot_clean);
    printf("\n-- evicted_hot_dirty: %d", count_evicted_hot_dirty);
    */

    if (victim == NULL){
        printf("\n-------------------------------------------------------");
     
        
        printf("\n[ERR0R]: No victim found OPP:%d", operations);
        exit(1);
    }

    if(victim->node->list == cold_clean){
        count_evicted_cold_clean++;
    }
    if(victim->node->list == cold_dirty){
        count_evicted_cold_dirty++;
    }
    if(victim->node->list == hot_clean){
        count_evicted_hot_clean++;
    }
    if(victim->node->list == hot_dirty){
        count_evicted_hot_dirty++;
    }

    return victim;
}





void * analyze(void * arg){
   

    int write_counter = 0;
    int read_counter = 0;
    int count = 0;

    int read_count_hit = 0;
    int read_count_miss = 0;
    int read_count_ghost_hit = 0;

    int write_count_hit = 0;
    int write_count_miss = 0;
    int write_count_ghost_hit = 0;

    max_page_frequency_interval = 0;

    for (int i = 0; i < historic_size_in; i++){
        struct HistoricNodeIn * n = historic_in[i];
        if(n->file_id == -1) break;
        
        // READ and WRITE intensity
        if(n->dirty_flag == PAGE_DIRTY){
            write_counter++;

            if(n->is_hit == 1) write_count_hit++;
            if(n->is_miss== 1) write_count_miss++;
            if(n->is_ghost_hit== 1) write_count_ghost_hit++;

        }else{
            read_counter++;

            if(n->is_hit == 1) read_count_hit++;
            if(n->is_miss== 1) read_count_miss++;
            if(n->is_ghost_hit== 1) read_count_ghost_hit++;
        }

        if(n->references > max_page_frequency_interval){
            max_page_frequency_interval = n->references;
        }
        
      
        count++;
    }
    write_intensity =  ( (write_counter * 100.0 ) / (double) count ) / 100.0;
    read_intensity =   ( (read_counter * 100.0 ) / (double) count ) / 100.0;
    
  
    write_ghost_hit_intensity = ( (write_count_ghost_hit * 100.0 ) / (double) count) / 100.0;
    read_ghost_hit_intensity = ( (read_count_ghost_hit * 100.0 ) / (double) count) / 100.0;

    
    write_miss_intensity = ( (write_count_miss * 100.0 ) / (double) count) / 100.0;
    read_miss_intensity = ( (read_count_miss * 100.0 ) / (double) count) / 100.0;

    
    write_hit_intensity = ( (write_count_hit * 100.0 ) / (double) count) / 100.0;
    read_hit_intensity = ( (read_count_hit * 100.0 ) / (double) count) / 100.0;
    
    //printf("\n W:%f R:%f", write_intensity, read_intensity);
    return NULL;
}

int postion_IN = 0;

void add_historic_in(struct Page * page, double references, int is_hit, int is_miss, int is_ghost_hit){
    if(postion_IN == historic_size_in){
        postion_IN = 0; //reset
    }
    struct HistoricNodeIn * h_node = historic_in[postion_IN];
    h_node->file_id = page->file_id;
    h_node->block_id = page->block_id;
    h_node->dirty_flag = page->dirty_flag;
    h_node->is_hit = is_hit;
    h_node->is_miss = is_miss;
    h_node->is_ghost_hit = is_ghost_hit;
    h_node->references = references;
    postion_IN ++;
    
}

void add_historic_out(struct MLNode * ml_node, struct HistoricNodeOut * ghost){
    struct Node * free_n = NULL;

    if(ghost == NULL){
        free_n = list_remove_tail(historic_list_out);
    }else{
        free_n = list_remove(historic_list_out, ghost->node);
    }


    struct HistoricNodeOut * h_node = (struct HistoricNodeOut *) free_n->content;
    
    hash_table_free_entity(
        hash_table_remove(ghost_hash, ( (unsigned int) h_node->block_id ))
    );

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
    hash_table_put(ghost_hash, ( (unsigned int) h_node->block_id ), h_node);
}


void NEW_insert(struct List * list, struct MLNode * ml_node){
    list_insert_node_head(list,ml_node->node);
}


double reward(double x){
    return MIN(x, 100.0);
}

double punish(double x){
    return MAX(x, 0.0);
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
    printf("\nIN:");
    /* TO DO
    struct Node * x = historic_list_in->tail;
    while(x!=NULL){
        struct HistoricNodeIn * n = (struct HistoricNodeIn *) x->content;
        printf(" %c[%d-%ld]",n->dirty_flag, n->file_id, n->block_id);
        x = x->prev;
    }
    */
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
    /*
    struct Node * x = historic_list_out->tail;

    while(x!=NULL){
        struct HistoricNodeOut * n = (struct HistoricNodeOut *) x->content;
        if(n->file_id == file_id && n->block_id == block_id){
            
            return n;
        }
        x = x->prev;
    }
    return NULL;
    */
    struct Entity * entity = hash_table_get(ghost_hash, ( (unsigned int) block_id ));
    if(entity == NULL){
        return NULL;
    }
    return (struct HistoricNodeOut *) entity->value;
}

#endif
