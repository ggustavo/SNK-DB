#ifndef DATA_BLOCK_H_INCLUDED
#define DATA_BLOCK_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "data_handle.h"

/*
 * Block layout defines how the data will be organized in the set of bytes
 * The set of bytes is in the range [0 to BLOCK_SIZE].
 *
 _______________________________________________________________________________
| 00   01   02   03 | 04   05   06   07 | 08   09   10   11 | 12   13   14   15 |
|     block_id      |     used_size     |        LSN        |  last_tuple_index |
|-------------------------------------------------------------------------------|
| 16   17   18   19 | 20   21   22   23 | 24   25   26   27 | 28   29   30   31 |
| tuples_index_size |   Tuple Index[0]  |        ...        |  Tuple Index[N]   |
---------------------------------------------------------------------------------
| 32   33   34   35   36   37   38   39   40 | 41   42   43   44   45   46   47 |
| <------------------------------- Free Space --------------------------------->|
---------------------------------------------------------------------------------
| 48   49   50   51   52 | 53   54   55   56   57   58   59   ...    BLOCK_SIZE |
| <---- Last Tuple ----> |        ...      | <---- First Tuple ---->|           |
---------------------------------------------------------------------------------
                               Data Block Layout
*
* The Header contains important metadata about the data block
* Currently composed of 5 metadata:
*
*   					BLOCK_ID => 4 bytes
*                      USED_SIZE => 4 bytes
*                           LSN  => 4 bytes
*                  BLOCK_STATUS  => 4 bytes
*             TUPLES_INDEX_SIZE  => 4 bytes
*                                  20 bytes  <----  HEADER TOTAL SIZE
*/

#define OFFSET_BLOCK_ID 0            //  [00 byte to 03 byte]
#define OFFSET_USED_SIZE 4           //  [04 byte to 07 byte]
#define OFFSET_LSN 8                 //  [08 byte to 11 byte]
#define OFFSET_LAST_TUPLE_INDEX 12   //  [12 byte to 15 byte]
#define OFFSET_TUPLES_INDEX_SIZE 16  //  [16 byte to 19 byte]

#define HEADER_SIZE 20

struct BlockHeader{
	int block_id;
	int used_size;                //space used by tuples
	int lsn;                      //The log sequence number (LSN) used in the recovery process
	int last_tuple_index;
	int tuple_index_size;         //number of tuples in block
};


struct BlockHeader * block_create_header(){
	struct BlockHeader * header = (struct BlockHeader *) malloc(sizeof(struct BlockHeader));
	header->block_id =          -1;
	header->used_size =         -1;
	header->lsn =               -1;
	header->last_tuple_index = -1;
	header->tuple_index_size =  -1;

	return header;
}

/*
 * Read char * data content block header and returns in a structure format (struct BlockHeader).
 */
struct BlockHeader * block_read_header(struct BlockHeader * header, char * data){
	header->block_id =                 read_int(data, OFFSET_BLOCK_ID);
	header->used_size =                read_int(data, OFFSET_USED_SIZE);
	header->lsn =    			       read_int(data, OFFSET_LSN);
	header->last_tuple_index =         read_int(data, OFFSET_LAST_TUPLE_INDEX);
	header->tuple_index_size =         read_int(data, OFFSET_TUPLES_INDEX_SIZE);

	return header;
}

/*
 * Write struct BlockHeader content in data block.
 */

struct BlockHeader* block_write_header(struct BlockHeader * header, char * data) {
	write_int(data, header->block_id,          OFFSET_BLOCK_ID);
	write_int(data, header->used_size,         OFFSET_USED_SIZE);
	write_int(data, header->lsn,               OFFSET_LSN);
	write_int(data, header->last_tuple_index,  OFFSET_LAST_TUPLE_INDEX);
	write_int(data, header->tuple_index_size,  OFFSET_TUPLES_INDEX_SIZE);

	return header;
}

void write_tuple(struct BlockHeader * header, char * data, char * tuple){

}


#endif
