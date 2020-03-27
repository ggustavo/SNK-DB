/*
#include <stdio.h>
#include <stdlib.h>
#include "../dbms/util/bloom_filter.h"
#include "../dbms/util/hash_functions.h"
#include <time.h>



int main(void) {

	
    int r = rand();
    srand(time(NULL));

    int * pointer = &r;
    int key = -1;
    int count = 0;
    int count_2 = 0;
    int res = -1;

    int bf_size = 100000000;
    int inserts = 100000000; 

    struct BloomFilter * bf = bloom_filter_create(bf_size);
    printf("\nsigle bitmap cost: %d bytes", bf->bitmap1->bytes);
    
    //r = 14050205;
    //bloom_filter_set(bf, (char*)pointer, 4);

    //printf("\nfind %d = %d", r, bloom_filter_get(bf,(char*)pointer, 4));

   // r = 3452378;
   // printf("\nfind %d = %d", r, bloom_filter_get(bf,(char*)pointer, 4));
    
   // r = 689023;
   // printf("\nfind %d = %d", r, bloom_filter_get(bf,(char*)pointer, 4));
    
   // r = 1048943;
   // printf("\nfind %d = %d", r, bloom_filter_get(bf,(char*)pointer, 4));
    
    int false_positives = 0;

    for(int i = 0; i < inserts; i++){
        //r = rand() * 13 * rand() * 7;
        r = i;

        res = bloom_filter_get(bf,(char*)pointer, 4);
        if(res == 0)false_positives ++;
        bloom_filter_set(bf,(char*)pointer, 4);
        
        //printf("\nfind %u = %d", r, res);


        if(count == inserts/100){
            count_2++;
            printf("\n------ %d %% ------ ", count_2);
            fflush(stdout);
            count = 0;
        }
        count ++;
    }
    double v1 = false_positives / 1.0;
    double v2 = inserts / 1.0;

    printf("\n\nInserts: %d", inserts );
    printf("\nFalse Positives: %f %%", (v1*100)/v2 );
    printf("\nBloom Filter cost: %d bytes", bf->bitmap1->bytes*3);
}

*/