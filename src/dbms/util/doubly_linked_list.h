#ifndef DLLIST_H_INCLUDED
#define DLLIST_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

struct List{
	 struct Node* head;
	 struct Node* tail;
	 int size;
	 void (*print_function)(void*);
	 void (*free_function)(void*);
};


struct Node {
    void* data;
    struct Node* next;
    struct Node* prev;
};

struct List* list_create(void (*print_function)(void*), void (*free_function)(void*)){
	struct List* list = (struct List*) malloc(sizeof(struct List));
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->print_function = print_function;
	list->free_function = free_function;
	return list;
}

struct Node * list_create_node(void* data){
	struct Node * new_node = (struct Node*) malloc(sizeof(struct Node));
	new_node->next = NULL;
	new_node->prev = NULL;
	new_node->data = data;
	return new_node;
}

void list_insert_head(struct List *list, void *data) {
	struct Node *x = list_create_node(data);

	if (list->head == NULL) {
		list->head = x;
		list->tail = x;
	} else {
		x->next = list->head;
		list->head->prev = x;
		list->head = x;
	}
	list->size++;
}

void list_insert_tail(struct List *list, void *data) {
	struct Node *x = list_create_node(data);

	if (list->head == NULL) {
		list->head = x;
		list->tail = x;
	} else {
		list->tail->next = x;
		x->prev = list->tail;
		list->tail = x;
	}
	list->size++;
}

struct Node * list_remove(struct List * list, struct Node*x) {
	if (x != NULL) {
		if (list->tail == list->head) {
			list->tail = list->head = NULL;
		} else if (x->next != NULL && x->prev != NULL) {
			x->prev->next = x->next;
			x->next->prev = x->prev;
		} else if (x == list->head) {
			list->head = x->next;
			list->head->prev = NULL;
		} else if (x == list->tail) {
			list->tail = x->prev;
			list->tail->next = NULL;
		}
		list->size--;
		x->next = NULL;
		x->prev = NULL;
	}
	return x;
}

struct Node * list_remove_head(struct List * list) {
	return list_remove(list,list->head);
}

struct Node * list_remove_tail(struct List * list) {
	return list_remove(list,list->tail);
}

struct Node * list_find_node(struct List * list, void* data, int(*compare_function)(void*,void*)) {

	struct Node * x = list->head;
	while(x != NULL){
		if(compare_function(data,x->data)){
			return x;
		}
		x = x->next;
	}
	return NULL;
}

void * list_find_data(struct List * list, void* element, int(*compare_function)(void*,void*)) {
	return list_find_node(list,element,compare_function)->data;
}

void list_print(struct List* list){
	struct Node * x = list->head;
	while(x != NULL){
		list->print_function(x->data);
		x = x->next;
	}
}
void list_print_inverse(struct List* list){
	struct Node * x = list->tail;
	while(x != NULL){
		list->print_function(x->data);
		x = x->prev;
	}
}

static void list_free_node(struct List* list, struct Node * x){
	x->next = NULL;
	x->prev = NULL;
	list->free_function(x->data);
	x->data = NULL;
	free(x);
}


#endif
