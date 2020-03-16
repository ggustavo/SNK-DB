/*
 * A basic implementation of a Static Hash Table
 */

#ifndef SIMPLE_HASH
#define SIMPLE_HASH

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
    int key;
    void * value;
};

struct Hash{
    int number_of_buckets;
    struct List ** buckets;
    int (*custom_hash_function)(struct Hash *,int);
    int size;
};


int simple_hash_function(struct Hash * hash, int key){
    return key % hash->number_of_buckets;
}

struct Hash * simple_hash_create(int number_of_buckets, int (*custom_hash_function)(struct Hash *,int)){
    struct Hash * hash = (struct Hash*) malloc(sizeof(struct Hash));
    hash->number_of_buckets = number_of_buckets;

    hash->buckets = (struct List**) malloc (sizeof(struct List*) * number_of_buckets);
    if(custom_hash_function == NULL){
        hash->custom_hash_function = simple_hash_function;
    }else{
        hash->custom_hash_function = custom_hash_function;
    }
    for (int i = 0; i < number_of_buckets; i++) {
        hash->buckets[i] = NULL;
    }

    return hash;
}

struct Entity * simple_hash_create_entity(int key, void * value){
    struct Entity * entity = (struct Entity*) malloc (sizeof(struct Entity));
    entity->key = key;
    entity->value = value;
    return entity;
}


void simple_hash_free_entity(struct Entity * entity){
    entity->value = NULL;
    free(entity);
}

/*
* Warning: this function can create duplicate keys. Use the simple_hash_update() function to avoid this.
*/
void simple_hash_put(struct Hash * hash, int key, void* value){
    int bucket_index = hash->custom_hash_function(hash, key);
    struct List * bucket = hash->buckets[ bucket_index ];
    if(bucket == NULL){
        bucket = hash->buckets[ bucket_index ] = list_create(NULL,NULL);
    }
    list_insert_tail(bucket, simple_hash_create_entity(key,value));
    hash->size++;
}

/*
* Creates a new entity if it does not exist.
*/
struct Entity * simple_hash_update(struct Hash * hash, int key, void* value){
    int bucket_index = hash->custom_hash_function(hash, key);
    struct List * bucket = hash->buckets[ bucket_index ];
    if(bucket == NULL){
        bucket = hash->buckets[ bucket_index ] = list_create(NULL,NULL);
        list_insert_tail(bucket, simple_hash_create_entity(key,value));
        hash->size++;
    }else{
        struct Node * x = bucket->head;
	    while(x != NULL){
		    struct Entity * entity = (struct Entity *) x->content;
            if(entity->key == key){
                x->content = simple_hash_create_entity(key, value);
                return entity; 
            } 
		    x = x->next;
	    }
    }
    return NULL;
}

struct Entity * simple_hash_get(struct Hash * hash, int key){
    int bucket_index = hash->custom_hash_function(hash, key);
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

struct Entity * simple_hash_remove(struct Hash * hash, int key){
    int bucket_index = hash->custom_hash_function(hash, key);
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

/*
* Converts all entities in the hash to a struct List.
* Return NULL if there is no entities.
*/
struct List * simple_hash_to_list(struct Hash * hash){
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


void simple_hash_print( struct Hash * hash, int(*print_function)(int,void*)){
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


#endif
