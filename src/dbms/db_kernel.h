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
	#define BUFFER_POLICY_NAME  "LRU"
#elif MRU
	#include "buffer_manager/policies/MRU.h"
	#define BUFFER_POLICY_NAME  "MRU"
#elif FIFO
	#include "buffer_manager/policies/FIFO.h"
	#define BUFFER_POLICY_NAME  "FIFO"
#elif CLOCK
	#include "buffer_manager/policies/CLOCK.h"
	#define BUFFER_POLICY_NAME  "CLOCK"
#elif GCLOCK
	#include "buffer_manager/policies/GCLOCK.h"
	#define BUFFER_POLICY_NAME  "GCLOCK"
#elif LFU
	#include "buffer_manager/policies/LFU.h"
	#define BUFFER_POLICY_NAME  "LFU"
#elif LFUDA
	#include "buffer_manager/policies/LFUDA.h"
	#define BUFFER_POLICY_NAME  "LFUDA"
#elif ARC
	#include "buffer_manager/policies/ARC.h"
	#define BUFFER_POLICY_NAME  "ARC"
#elif MQ
	#include "buffer_manager/policies/MQ.h"
	#define BUFFER_POLICY_NAME  "MQ"
#elif FBR
	#include "buffer_manager/policies/FBR.h"
	#define BUFFER_POLICY_NAME  "FBR"
#elif LRUMIS
	#include "buffer_manager/policies/LRUMIS.h"
	#define BUFFER_POLICY_NAME  "LRUMIS"
#elif F2Q
	#include "buffer_manager/policies/F2Q.h"
	#define BUFFER_POLICY_NAME  "F2Q"
#elif LRUK
	#include "buffer_manager/policies/LRUK.h"
	#define BUFFER_POLICY_NAME  "LRUK"
#elif LIRS
	#include "buffer_manager/policies/LIRS.h"
	#define BUFFER_POLICY_NAME  "LIRS"
#elif CFLRU
	#include "buffer_manager/policies/CFLRU.h"
	#define BUFFER_POLICY_NAME  "CFLRU"
#elif LRUWSR
	#include "buffer_manager/policies/LRUWSR.h"
	#define BUFFER_POLICY_NAME  "LRUWSR"
#elif CCFLRU
	#include "buffer_manager/policies/CCFLRU.h"
	#define BUFFER_POLICY_NAME  "CCFLRU"
#elif CCCFLRU
	#include "buffer_manager/policies/CCCFLRU.h"
	#define BUFFER_POLICY_NAME  "CCCFLRU"
#elif CFDC
	#include "buffer_manager/policies/CFDC.h"
	#define BUFFER_POLICY_NAME  "CFDC"
#elif CASA
	#include "buffer_manager/policies/CASA.h"
	#define BUFFER_POLICY_NAME  "CASA"
#elif ADLRU
	#include "buffer_manager/policies/ADLRU.h"
	#define BUFFER_POLICY_NAME  "ADLRU"
#elif LLRU
	#include "buffer_manager/policies/LLRU.h"
	#define BUFFER_POLICY_NAME  "LLRU"
#elif AMLRU
	#include "buffer_manager/policies/AMLRU.h"
	#define BUFFER_POLICY_NAME  "AMLRU"
#elif GASA
	#include "buffer_manager/policies/GASA.h"
	#define BUFFER_POLICY_NAME  "GASA"
#elif SCMBP
	#include "buffer_manager/policies/SCMBP.h"
	#define BUFFER_POLICY_NAME  "SCMBP"
#elif ML1
	#include "buffer_manager/policies/ML1.h"
	#define BUFFER_POLICY_NAME  "ML1"

#elif ML2
	#include "buffer_manager/policies/ML2.h"
	#define BUFFER_POLICY_NAME  "ML2"	

#elif ML3
	#include "buffer_manager/policies/ML3.h"
	#define BUFFER_POLICY_NAME  "ML3"	

#elif ML4
	#include "buffer_manager/policies/ML4.h"
	#define BUFFER_POLICY_NAME  "ML4"	

#else /* If no policy has been chosen. Use LRU as default */
	#include "buffer_manager/policies/LRU.h"
	#define BUFFER_POLICY_NAME  "LRU"
#endif


void start_database(){
	catalog_start();
	buffer_start();
	buffer_policy_start();
}

#endif
