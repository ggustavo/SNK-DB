/*
#include <stdio.h>
#include <stdlib.h>

#include "../db_config.h"
#include "../dbms/util/doubly_linked_list.h"


void print_l(void* data){
	printf("%d, ",(int)data);
}

int compare_n(void * data, void *data_node){
	if((int)data == (int)data_node){
		return 1;
	}
	return 0;
}

int main(void) {

	struct List * list = list_create(print_l,NULL);

	list_insert_head(list,12);
	list_print(list);
	printf("\n");

	list_insert_tail(list,13);
	list_print(list);
	printf("\n");

	list_insert_tail(list,14);
	list_print(list);
	printf("\n");

	list_insert_tail(list,15);
	list_print(list);
	printf("\n");

	list_insert_head(list,11);
	list_print(list);
	printf("\n");

	list_insert_head(list,10);
	list_print(list);
	printf("\n");

	list_print_inverse(list);
	printf("\n");


	struct Node * x = NULL;

	x = list_find_node(list,60,compare_n);
		if(x==NULL){printf("NULL\n");}else{printf("find %d\n",x->data);}

	x = list_find_node(list,13,compare_n);
		if(x==NULL){printf("NULL\n");}else{printf("find %d\n",x->data);}

	x = list_find_node(list,12,compare_n);
		if(x==NULL){printf("NULL\n");}else{printf("find %d\n",x->data);}

	list_remove_head(list);
	list_print(list);
	printf("\n");

	list_remove_tail(list);
	list_print(list);
	printf("\n");

	list_remove(list,x);
	list_print(list);
	printf("\n");

}
*/
