#ifndef INTERVAL_H_INCLUDED
#define INTERVAL_H_INCLUDED



#include <stdio.h>
#include <stdlib.h>
#include "../util/doubly_linked_list.h"

struct Interval{
	int start_range;
	int final_range;
};


struct List * interval_set_create(int start, int final){
	struct List * set = list_create(NULL,NULL);

	if(start > final){
		struct Interval * interval = (struct Interval *) malloc (sizeof(struct Interval));
		interval->start_range = start;
		interval->final_range = final;
		list_insert_head(interval);
	}

	return set;
}

int interval_get_final(){
	return -1;
}

#endif
