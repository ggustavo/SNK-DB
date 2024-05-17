/*
#include <stdio.h>
#include <stdlib.h>
#include "../dbms/util/hash_table.h"
#include "../dbms/util/hash_functions.h"
#include <time.h>

void print_f(int key, void* value){
        printf(" %d[%d]->", key, value);
}

int main(void) {

    int hash_size = 16000000;
	struct Hash * hash = hash_table_create(hash_size);
    
    int r = rand();
    srand(time(NULL));
    int * pointer = &r;

    int inserts = 16000002; 
    
    int key = -1;
    struct Entity * e = NULL;
    


    int count = 0;
    int count_2 = 0;
    for(int i = 0; i < inserts; i++){
        r = rand();
        
       // key = hash_djb2((char*)pointer, 4);
        key = i;

        e = hash_table_update(hash, key, NULL);
        if(e != NULL) free(e);
        
        if(count == inserts/100){
            count_2++;
            printf("\n%d %%", count_2);
            fflush(stdout);
            count = 0;
        }
        count ++;
    }
    hash_table_size_of(hash);
    double v1 = hash->number_of_collisions / 1.0;
    double v2 = inserts / 1.0;

    printf("\ncollisions: %f %%",(v1*100)/v2);
 
}
*/