#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "db_config.h"

#include "buffer_manager/policies/LRU.h"

void start_database(){
	buffer_start();
	buffer_policy_start();
}

#endif
