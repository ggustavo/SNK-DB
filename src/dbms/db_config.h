#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

// -----> Buffer Manager Setting --------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// ###################################################################
int BLOCK_SIZE  = 100;   // Sets the size of the block accessed on the secondary storage media
int BUFFER_SIZE = 5;    // Sets the number of pages resident in the buffer.
//###################################################################
//--------------------------------------------------------------------------------------------


// -----> Block Header Settings ---------------------------------------------------------------
// ############################################################################################
#define OFFSET_BLOCK_ID 0            //  [00 byte to 03 byte]
#define OFFSET_USED_SIZE 4           //  [04 byte to 07 byte]
#define OFFSET_LSN 8                 //  [08 byte to 11 byte]
#define OFFSET_STATUS 12             //  [12 byte to 15 byte]
#define OFFSET_TUPLES_INDEX_SIZE 16  //  [16 byte to 19 byte]
#define HEADER_SIZE 20
#define OFFSET_FIRST_ENTITY_INDEX 20

#define INDEX_ENTITY_BYTES 4 // To store one index entity is required 4 bytes
// ############################################################################################
// --------------------------------------------------------------------------------------------



// -----> Definitions of System ---------------------------------------------------------------
// ############################################################################################
#define FALSE '0'
#define  TRUE '1'

/*
 * STATUS_LOCKED -> When the query processor is performing an operation on the page (reading or writing),
 * otherwise: STATUS_UNLOCKED
 */
#define PAGE_STATUS_LOCKED 'L'
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
#define READ_REQUEST 'R'
// ############################################################################################
// --------------------------------------------------------------------------------------------


// -----> Errors and Returns ------------------------------------------------------------------
// ############################################################################################
#define ERROR_BLOCK_IS_FULL -1
#define SUCCESS_WRITE_TUPLE 2
#define ERROR_TUPLE_GREATER_THAN_BLOCK_SIZE -3
// ############################################################################################
// --------------------------------------------------------------------------------------------


#endif
