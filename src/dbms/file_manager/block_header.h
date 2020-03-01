#ifndef DATA_BLOCK_HEADER_H_INCLUDED
#define DATA_BLOCK_HEADER_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "data_handle.h"
#include "../db_config.h"

/*
 * Block layout defines how the data will be organized in the set of bytes
 * The set of bytes is in the range [0 to BLOCK_SIZE].
 *
 _______________________________________________________________________________
| 00   01   02   03 | 04   05   06   07 | 08   09   10   11 | 12   13   14   15 |
|     block_id      |     used_size     |        LSN        |       status      |
|-------------------------------------------------------------------------------|
| 16   17   18   19 | 20   21   22   23 | 24   25   26   27 | 28   29   30   31 |
| tuples_index_size |   Tuple Index[0]  |        ...        |  Tuple Index[N]   |
---------------------------------------------------------------------------------
| 32   33   34   35   36   37   38   39   40 | 41   42   43   44   45   46   47 |
| <------------------------------- Free Space --------------------------------->|
---------------------------------------------------------------------------------
| 48   49   50   51   52 | 53   54   55   56   57   58   59   ...    BLOCK_SIZE |
| <---- Last Tuple ----> |        ...      | <------- First Tuple ------->|     |
---------------------------------------------------------------------------------
                               Data Block Layout
*
* The Header contains important metadata about the data block
* Currently composed of 5 metadata:
*
*   					BLOCK_ID  [00 byte to 03 byte] => 4 bytes
*                      USED_SIZE  [04 byte to 07 byte] => 4 bytes
*                            LSN  [08 byte to 11 byte] => 4 bytes
*              FIRST_TUPLE_INDEX  [12 byte to 15 byte] => 4 bytes
*              TUPLES_INDEX_SIZE  [16 byte to 19 byte] => 4 bytes
*                                  						 20 bytes  <----  HEADER TOTAL SIZE
*/

struct BlockHeader{
	int block_id;
	int used_size;                // Space used by tuples
	int lsn;                      // The log sequence number (LSN) used in the recovery process
	int status;
	int tuple_index_size;         // Number of tuples in block
};


struct BlockHeader * header_create(){
	struct BlockHeader * header = (struct BlockHeader *) malloc(sizeof(struct BlockHeader));
	header->block_id =           -1;
	header->used_size =           0;
	header->lsn =                -1;
	header->status =             -1;
	header->tuple_index_size =    0;

	return header;

}

/*
 * Read char * data content block header and returns in a structure format (struct BlockHeader).
 */
struct BlockHeader * header_read(struct BlockHeader * header, char * data){
	header->block_id =                 handler_read_int(data, OFFSET_BLOCK_ID);
	header->used_size =                handler_read_int(data, OFFSET_USED_SIZE);
	header->lsn =    			       handler_read_int(data, OFFSET_LSN);
	header->status =                   handler_read_int(data, OFFSET_STATUS);
	header->tuple_index_size =         handler_read_int(data, OFFSET_TUPLES_INDEX_SIZE);

	return header;
}

/*
 * Write struct BlockHeader content in data block.
 */

struct BlockHeader* header_write(struct BlockHeader * header, char * data) {
	handler_write_int(data, header->block_id,          OFFSET_BLOCK_ID);
	handler_write_int(data, header->used_size,         OFFSET_USED_SIZE);
	handler_write_int(data, header->lsn,               OFFSET_LSN);
	handler_write_int(data, header->status,            OFFSET_STATUS);
	handler_write_int(data, header->tuple_index_size,  OFFSET_TUPLES_INDEX_SIZE);

	return header;
}



void header_print(struct BlockHeader * header){
	int free_space = BLOCK_SIZE - ( HEADER_SIZE + (header->tuple_index_size * INDEX_ENTITY_BYTES) + header->used_size );

	printf("\n------------------------- Block ID: %d, LSN: %d", header->block_id, header->lsn);
	printf("\nUsed Size by Tuples: %d", header->used_size);
	printf("\nTuple Index Size: %d, Status: byte-> %d",header->tuple_index_size, header->status);
	printf("\nFree Space: %d", free_space);
	printf("\n--------------------------------------");
}


#endif
