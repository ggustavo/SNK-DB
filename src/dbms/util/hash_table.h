/*
 * A basic implementation of a Static Hash Table
 */

#ifndef HASH_TABLE
#define HASH_TABLE

#include <stdlib.h>
#include <stdio.h>
#include "doubly_linked_list.h"

/*
* This simple hash table uses a fixed-size array of pointers (struct List ** buckets).
* Each element of the array points to a linked list. 
* In case of collision, the new entity is inserted at the end of the list.           
* 
*                -----------------
*                | struct Entity |     
*  buckets[0]--> |  key, *value  |     
*                |    overflow---|---> NULL
*                -----------------

*                -----------------      -----------------      -----------------
*                | struct Entity |      | struct Entity |      | struct Entity |
*  buckets[1]--> |  key, *value  |      |  key, *value  |      |  key, *value  |
*                |    overflow---|--->  |    overflow---|--->  |    overflow---|-->
*                -----------------      -----------------      -----------------
*       .                       (collision)            (collision)
*       .
*       .
*                -----------------
*                | struct Entity |
*  buckets[N]--> |  key, *value  |
*                |    overflow---|---> NULL
*                -----------------
*/

struct Entity{
    unsigned int key;
    void * value;
};

struct Hash{
    int number_of_buckets;
    struct List ** buckets;
    int size;
    int number_of_collisions;
};

int hash_table_function(struct Hash * hash, unsigned int key){
    return key % hash->number_of_buckets;
}

struct Hash * hash_table_create(int number_of_buckets){
    struct Hash * hash = (struct Hash*) malloc(sizeof(struct Hash));
    hash->number_of_buckets = number_of_buckets;
    hash->number_of_collisions = 0;
    hash->size = 0;
    hash->buckets = (struct List**) malloc (sizeof(struct List*) * number_of_buckets);
    
    for (int i = 0; i < number_of_buckets; i++) {
        hash->buckets[i] = NULL;
    }

    return hash;
}

struct Entity * hash_table_create_entity(unsigned int key, void * value){
    struct Entity * entity = (struct Entity*) malloc (sizeof(struct Entity));
    entity->key = key;
    entity->value = value;
    return entity;
}


void hash_table_free_entity(struct Entity * entity){
    if(entity != NULL){
        entity->value = NULL;
        free(entity);
    }
}

/*
* Warning: this function can create duplicate keys. Use the hash_table_update() function to avoid this.
*/
void hash_table_put(struct Hash * hash, unsigned int key, void* value){
    int bucket_index = hash_table_function(hash, key);
    struct List * bucket = hash->buckets[ bucket_index ];
    if(bucket == NULL){
        bucket = hash->buckets[ bucket_index ] = list_create(NULL,NULL);
    }else{
        hash->number_of_collisions++;
    }
    list_insert_tail(bucket, hash_table_create_entity(key,value));
    hash->size++;
}

/*
* Creates a new entity if it does not exist and return the previous entity if it exists
*/
struct Entity * hash_table_update(struct Hash * hash, unsigned int key, void* value){
    int bucket_index = hash_table_function(hash, key);
    struct List * bucket = hash->buckets[ bucket_index ];
    if(bucket == NULL){
        bucket = hash->buckets[ bucket_index ] = list_create(NULL,NULL);
        list_insert_tail(bucket, hash_table_create_entity(key,value));
        hash->size++;
    }else{
        hash->number_of_collisions++;
        struct Node * x = bucket->head;
	    while(x != NULL){
		    struct Entity * entity = (struct Entity *) x->content;
            if(entity->key == key){
                x->content = hash_table_create_entity(key, value);
                return entity; 
            } 
		    x = x->next;
	    }
    }
    return NULL;
}

struct Entity * hash_table_get(struct Hash * hash, unsigned int key){
    int bucket_index = hash_table_function(hash, key);
    struct List * bucket = hash->buckets[ bucket_index ];
    if(bucket == NULL) return NULL;
    struct Node * x = bucket->head;
	while(x != NULL){
		struct Entity * entity = (struct Entity *) x->content;
        if(entity->key == key) return entity;
		x = x->next;
	}
    return NULL;
} 

struct Entity * hash_table_remove(struct Hash * hash, unsigned int key){
    int bucket_index = hash_table_function(hash, key);
    struct List * bucket = hash->buckets[ bucket_index ];
    if(bucket == NULL) return NULL;
    struct Node * x = bucket->head;
	while(x != NULL){
		struct Entity * entity = (struct Entity *) x->content;
        if(entity->key == key){
            struct Node * node = list_remove(bucket, x);
            list_free_node(bucket, node);
            hash->size--;
            return entity;  
        } 
		x = x->next;
	}
    return NULL;
} 

void hash_table_free(struct Hash * hash){
    for (int i = 0; i < hash->number_of_buckets; i++) {
        struct List * bucket = hash->buckets[i];
        if(bucket!=NULL){
            if (bucket->size > 0){
                printf("\n[WARN] hash_table_free: bucket[%d] is not empty", i);
            }
            list_free(bucket);
        }
        hash->buckets[i] = NULL;
    }
    free(hash->buckets);
    free(hash);
}


/*
* Converts all entities in the hash to a struct List.
* Return NULL if there is no entities.
*/
struct List * hash_table_to_list(struct Hash * hash){
    if(hash->size == 0){
        return NULL;
    }
    struct List * list = list_create(NULL,NULL);
    for (int i = 0; i < hash->number_of_buckets; i++) {
        struct List * bucket = hash->buckets[i];
        if(bucket!=NULL){
            struct Node * x = bucket->head;
	        while(x != NULL){
		        struct Entity * entity = (struct Entity *) x->content;
                list_insert_tail(list, entity);
		        x = x->next;
	        }
        }
    }    
    return list;
}


void hash_table_print( struct Hash * hash, int(*print_function)(int,void*)){
    for (int i = 0; i < hash->number_of_buckets; i++) {
        struct List * bucket = hash->buckets[i];
        if(bucket!=NULL){
            printf("\nbucket[%d]->",i);
            struct Node * x = bucket->head;
	        while(x != NULL){
		        struct Entity * entity = (struct Entity *) x->content;
                print_function(entity->key, entity->value);
		        x = x->next;
	        }
        }
    }
}


long hash_table_size_of(struct Hash * hash){
    unsigned long long size = 0;
    
    size+= sizeof(struct Hash);
    size+= sizeof(struct List*)   * hash->number_of_buckets;
    size+= sizeof(struct List)    * hash->number_of_buckets;
    size+= sizeof(struct Node)    * hash->size;
    size+= sizeof(struct Entity)  * hash->size;
    
    printf("\nsize of struct Hash:   %d", sizeof(struct Hash));
    printf("\nsize of struct Entity: %d", sizeof(struct Entity));
    printf("\nsize of struct List:   %d", sizeof(struct List));
    printf("\nsize of struct Node:   %d", sizeof(struct Node));
    printf("\nnumber of buckets:     %d", hash->number_of_buckets);
    printf("\nnumber of collisions:  %d", hash->number_of_collisions);
    printf("\nnumber of elements:    %d", hash->size);
    printf("\ntotal size (bytes):    %lu", size);

    return size;
    
}

#endif
