/*
 * db_kernel.h starts in the correct order all services or components of the system.
 */

#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "db_config.h"
#include "file_manager/catalog.h"

/*
* These conditional macros choose the Page Replacement Policy
*
* You can choose a policy by adding the macro: "#define POLICY_NAME TRUE"
* for example:
*				#define MRU TRUE // <----- Choose the MRU
*		
*				#if LRU
*					....   //<----- ignored
*				#elif MRU 
*					...    //<----- Enter here! and include the MRU.h file!
* --------------------------------------------------------------------------------------------
*
* Another way to choose the policy is to add the "-DPOLICY_NAME" option in the compilation phase
* for example:
*				$gcc [others parameters here...] -DFIFO

* "-DFIFO" makes the compiler add a new macro (like #define FIFO)
* --------------------------------------------------------------------------------------------
*
* To add a NEW POLICY just add a new "#elif" ("else if")
* for example:
* 				#elif POLICY_NAME
					#include "buffer_manager/policies/POLICY_NAME.h"
*/

#ifdef LRU
	#include "buffer_manager/policies/LRU.h"	
#elif MRU
	#include "buffer_manager/policies/MRU.h"
#elif FIFO
	#include "buffer_manager/policies/FIFO.h"
#elif CLOCK
	#include "buffer_manager/policies/CLOCK.h"
#elif GCLOCK
	#include "buffer_manager/policies/GCLOCK.h"
#elif LFU
	#include "buffer_manager/policies/LFU.h"
#elif LFUDA
	#include "buffer_manager/policies/LFUDA.h"
#elif ARC
	#include "buffer_manager/policies/ARC.h"
#elif MQ
	#include "buffer_manager/policies/MQ.h"

#else /* If no policy has been chosen. Use LRU as default */
	#include "buffer_manager/policies/LRU.h"
#endif


void start_database(){
	catalog_start();
	buffer_start();
	buffer_policy_start();
}

#endif
