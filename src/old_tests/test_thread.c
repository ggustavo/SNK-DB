/*
#include <stdio.h>
#include <stdlib.h>
#include "../dbms/transaction_manager/transaction.h"


void * exec(void * args) { 
    
    struct Transaction * transaction = (struct Transaction *) args;
   
    pthread_mutex_lock(&transaction->mutex);
    
    printf("\n Transaction %d ", transaction->id); 
    fflush(stdout);

    for (int i = 0; i < 3; i++){
        printf(" %d(%d)",transaction->id, i); 
        fflush(stdout);
        transaction_wait(transaction);

    }
    
    pthread_mutex_unlock(&transaction->mutex);
    
    printf("\n Transaction %d FINISH!", transaction->id); 
    fflush(stdout);


    return NULL; 
}




int main(void) {

   
    printf("\nBefore Threads\n"); 
    
    struct Transaction * t1 = transaction_create(1);
    transaction_begin(t1, exec);

    //struct Transaction * t2 = transaction_create(2);
    //transaction_begin(t2, exec);

    //struct Transaction * t3 = transaction_create(3);
    //transaction_begin(t3, exec2);
    
    
    //pthread_join(t1->thread, NULL); 

    int chr;
    while(1){
        printf("\nEnter a character: ");
        fflush(stdout);
        scanf("%d",&chr);  
        if(chr== 1){
            transaction_notify(t1);
        } 
        if(chr==0){
            transaction_free(t1);
            break;
        }
    }

   // pthread_join(t2->thread, NULL); 
   // pthread_join(t3->thread, NULL); 
    
  

  // pthread_cond_signal(&cond);


    printf("\nAfter Threads\n"); 
    exit(0); 

}

*/