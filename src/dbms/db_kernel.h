/*
 * db_kernel.h starts in the correct order all services or components of the system.
 */

#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "db_config.h"

#include "buffer_manager/policies/LRU.h"
#include "file_manager/catalog.h"

void start_database(){
	catalog_start();
	buffer_start();
	buffer_policy_start();
}

#endif
