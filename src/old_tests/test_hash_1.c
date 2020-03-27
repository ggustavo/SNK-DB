/*
#include <stdio.h>
#include <stdlib.h>
#include "../dbms/util/simple_hash_table.h"

void print_f(int key, void* value){
        printf(" %d[%d]->", key, value);
}

int main(void) {

	struct Hash * hash = simple_hash_create(10, NULL);

 
    simple_hash_put(hash,21, 0);
    simple_hash_put(hash,22, 320);
    simple_hash_put(hash,23, 420);

    simple_hash_put(hash,21, 0);
    simple_hash_put(hash,22, 320);
    simple_hash_put(hash,23, 420);


    if(simple_hash_get(hash,21) == NULL){
        printf("ERROR?");
    }

    printf("\n%d value " , simple_hash_get(hash,21)->value );
    simple_hash_update(hash,21, 1);
    printf("\n%d value " , simple_hash_get(hash,21)->value );

    simple_hash_print(hash, print_f);

    simple_hash_remove(hash, 22);
    printf("\n test \n");

    simple_hash_print(hash, print_f);

    simple_hash_remove(hash, 22);
    printf("\n test \n");

    simple_hash_print(hash, print_f);

    struct List * list = simple_hash_to_list(hash);
    struct Node * x = list->head;
	while(x != NULL){
		struct Entity * entity = (struct Entity *) x->content;
        printf("\n %d[%d]",entity->key, entity->value);
		x = x->next;
	}
 
}

*/