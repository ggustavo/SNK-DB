/*
#include <stdio.h>
#include <stdlib.h>
#include "../dbms/util/bloom_filter.h"
#include "../dbms/util/hash_functions.h"
#include <time.h>



int main(void) {

    int hash_size = 1600;
	
    int r = rand();
    srand(time(NULL));

    int * pointer = &r;

    int inserts = 10; 
    
    int key = -1;

    


    int count = 0;
    int count_2 = 0;
    for(int i = 0; i < inserts; i++){
        r = rand() * 7 * rand();
        
          //key = i;
        //key = hash_djb2((char*)pointer, 4);

        printf("\n----- Key: %u-----------------------------------", r);
        printf("\ndjb2:      %u", hash_djb2((char*)pointer, 4));
        printf("\nadditive:  %u", hash_additive((char*)pointer, 4,  2147483647)); 
        printf("\nrotating:  %u", hash_rotating((char*)pointer, 4,  2147483647));;
        printf("\njenkins:   %u", hash_jenkins((char*)pointer, 4));
        printf("\npearson:   %u", hash_pearson((char*)pointer, 4));
        printf("\nsmbm:      %u", hash_sdbm((char*)pointer, 4));
        printf("\nlose lose: %u", hash_lose_lose((char*)pointer, 4));
       

        if(count == inserts/100){
            count_2++;
            printf("\n------ %d %% ------ ", count_2);
            fflush(stdout);
            count = 0;
        }
        count ++;
    }
}
*/