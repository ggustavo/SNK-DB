#ifndef TRANSACTION
#define TRANSACTION


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h> 

#define COMMITTED  'C'
#define   ABORTED  'A'
#define    FAILED  'F'

#define    ACTIVE  'T'
#define   STOPPED  'S'
#define   WAITING  'W'


struct Transaction{
    int id;
    pthread_t thread; 
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char status;
};


struct Transaction * transaction_create(int id){
    struct Transaction * transaction = (struct Transaction *) malloc (sizeof(struct Transaction));
    transaction->id = id;
    transaction->status = STOPPED;

    pthread_mutex_init(&transaction->mutex, NULL);
    pthread_cond_init(&transaction->cond, NULL);

    return transaction;
}

void transaction_free(struct Transaction * transaction){
    pthread_mutex_destroy(&transaction->mutex);
    pthread_cond_destroy(&transaction->cond);
    free(transaction);
}

void transaction_begin(struct Transaction * transaction, void *(* func)(void *)){
    transaction->status = ACTIVE;
    pthread_create(&transaction->thread, NULL, func, transaction); 
}

void transaction_abort(struct Transaction * transaction){
    transaction->status = ABORTED; // TODO 
}

void transaction_commit(struct Transaction * transaction){
    transaction->status = COMMITTED; // TODO 
}

void transaction_wait(struct Transaction * transaction){
    transaction->status = WAITING;
    pthread_cond_wait(&transaction->cond, &transaction->mutex);
}

void transaction_notify(struct Transaction * transaction){
    transaction->status = ACTIVE;
    pthread_cond_signal(&transaction->cond);
}

#endif