#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED


// -----> Simple Macros --------------------------------------------------------------
// --------------------------------------------------------------------------------------------
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define SAFE_DIVISION(a,b) ( ((b)==(0))?(0):(((double)a)/((double)b)) ) 
#define ABS(N) ((N<0)?(-N):(N))

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// -----> Buffer Manager Setting --------------------------------------------------------------
// --------------------------------------------------------------------------------------------

#ifndef BLOCK_SIZE
	#define BLOCK_SIZE  100  /* Sets the size of the block accessed on the secondary storage media */
#endif

#ifndef BUFFER_SIZE
	#define BUFFER_SIZE 5    /*  Sets the number of pages resident in the buffer. */
#endif

//--------------------------------------------------------------------------------------------

// -----> Catalog Setting --------------------------------------------------------------
// --------------------------------------------------------------------------------------------

char CATALOG_DATA_FOLDER[]    = "data";
char CATALOG_SCHEMAS_FOLDER[] = "data/schemas";

//--------------------------------------------------------------------------------------------


// -----> Block Header Settings ---------------------------------------------------------------

#define OFFSET_BLOCK_ID 0            /* Block Header Offset [00 byte to 03 byte] */ 
#define OFFSET_USED_SIZE 4           /* Block Header Offset [04 byte to 07 byte] */ 
#define OFFSET_LSN 8                 /* Block Header Offset [08 byte to 11 byte] */ 
#define OFFSET_STATUS 12             /* Block Header Offset [12 byte to 15 byte] */
#define OFFSET_TUPLES_INDEX_SIZE 16  /* Block Header Offset [16 byte to 19 byte] */
#define HEADER_SIZE 20
#define OFFSET_FIRST_ENTITY_INDEX 20

#define INDEX_ENTITY_BYTES 4 /* To store one index entity is required 4 bytes */

// --------------------------------------------------------------------------------------------



// -----> System Definitions ------------------------------------------------------------------

#define FALSE 0
#define  TRUE 1

/*
 * STATUS_LOCKED -> When the query processor is performing an operation on the page (reading or writing),
 * otherwise: STATUS_UNLOCKED
 */
#define PAGE_STATUS_LOCKED   'L'
#define PAGE_STATUS_UNLOCKED 'U'

/*
 * DIRTY -> When the page received write operations (insert, update, delete),
 * otherwise: CLEAN
 */
#define PAGE_DIRTY 'W'
#define PAGE_CLEAN 'R'

/*
 * The Query Processor indicates that it needs to READ or WRITE when requesting a page.
 */
#define WRITE_REQUEST 'W'
#define READ_REQUEST  'R'

// --------------------------------------------------------------------------------------------



// -----> Errors and Returns ------------------------------------------------------------------

#define ERROR_BLOCK_IS_FULL -1
#define SUCCESS_WRITE_TUPLE 2
#define ERROR_TUPLE_GREATER_THAN_BLOCK_SIZE -3

#define CREATED 'C'
#define OPENED  'O'

// --------------------------------------------------------------------------------------------

// -----> Compiler Macros ------------------------------------------------------------------

#ifdef WIN
	#include <io.h>       /* to use mkdir? windows? */
	#define xmkdir(path) mkdir(path); /* <----- windows? */
#else
	#define xmkdir(path) mkdir(path, 0777); /* -DWIN to compile on Windows (warning: is not stable) */
#endif

void debug(const char *format, ...) {
    va_list vl;
    va_start(vl, format);
    vprintf(format, vl);
    //printf(format, vl);
    va_end(vl);
}


#endif
