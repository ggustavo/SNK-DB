/*
 * General Purpose Bitmap (GPB)
 */

#ifndef GPBITMAP
#define GPBITMAP

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MASK_0  0x80 // 10000000
#define MASK_1  0x40 // 01000000
#define MASK_2  0x20 // 00100000
#define MASK_3  0x10 // 00010000
#define MASK_4  0x08 // 00001000
#define MASK_5  0x04 // 00000100
#define MASK_6  0x02 // 00000010
#define MASK_7  0x01 // 00000001

#define MASK_255 0xFF
/*
*            |-----------------------|           |-----|
*            |  and  |   or  |  xor  |           | not |
*    |-------|-------|-------|-------|           |-----|-----|
*    | a | b | a & b | a | b | a ^ b |           |  a  |  ~a |
*    |---|---|-------|-------|-------|           |-----|-----|
*    | 0 | 0 |   0   |   0   |   0   |           |  0  |  1  |
*    | 0 | 1 |   0   |   1   |   1   |           |  1  |  0  |
*    | 1 | 0 |   0   |   1   |   1   |           |-----------|
*    | 1 | 1 |   1   |   1   |   0   |
*    |-------------------------------|
*
*    struct Bitmap consists of an array of chars (char * bits) that represents the bits stored.
*    10 million elements should cost around 1.25 megabytes (10.000.000 / 8e+6)
*       
*            |--------bits[0]--------|     |--------bits[1]--------|       |---------------------bits[bytes-1]---------------------|
*            |00|01|02|03|04|05|06|07|     |00|01|02|03|04|05|06|07|  ...  |  00  |  01  |  02  |  03  |  04  |  05  |  06  |  07  |
*            |--|--|--|--|--|--|--|--|     |--|--|--|--|--|--|--|--|       |------|------|------|------|------|------|------|------|
* index =>   |00|01|02|03|04|05|06|07|     |08|09|10|11|12|13|14|15|       |size-8|size-7|size-6|size-5|size-4|size-3|size-2|size-1|                            
*
*    Example: set index = 10 to one:
*           1) Find bits[?]: 
*                bits[index=10 / 8], returns bits[1]    
*            2) Index 10 is at position 2, how to find this?
*               index % 8, returns 2
*            3) Preparing the mask:
*                    0x01 = |0|0|0|0|0|0|0|1|   we need to move 1 in position 7 to position 2
*                           |0|1|2|3|4|5|6|7|   need (7 - 2) left shifts (<<) 
*                                               thus, 0x01 << (7 - (index % 8)), returns |0|0|1|0|0|0|0|0|
*            4) applying the or operator
*                bitmap->bits[index / 8] | ( 0x01 << (7 - (index % 8)) ); 
*                
*                |0|0|0|0|0|0|0|0|  => bitmap->bits[index / 8]  
*              + |0|0|1|0|0|0|0|0|  => 0x01 << (7 - (index % 8))
*              -------------------
*                |0|0|1|0|0|0|0|0|   now index 10 is equal to 1!                            
*/
struct Bitmap{
    char * bits;
    size_t size;  // Number of Elements (bits)
    size_t bytes; // Size in BYTES stored in memory
};

struct Bitmap * bitmap_create(size_t size){
    struct Bitmap * bitmap = (struct Bitmap*) malloc(sizeof(struct Bitmap));
    bitmap->size = size;
    bitmap->bytes = ceil(size / 8.0);
    bitmap->bits = (char*) malloc (bitmap->bytes);
    return bitmap;
}

int bitmap_get(struct Bitmap * bitmap, size_t index){ 

    return bitmap->bits[index / 8]  >>  (7 - (index % 8) )  &  0x01; /* AND */
}

void bitmap_set(struct Bitmap * bitmap, size_t index){

    bitmap->bits[index / 8] = bitmap->bits[index / 8] | ( 0x01 << (7 - (index % 8) ) );  /* OR */
}

void bitmap_clear(struct Bitmap * bitmap, size_t index){
 
    bitmap->bits[index / 8] = bitmap->bits[index / 8] & ~( 0x01 << (7 - (index % 8) ) );
}

void bitmap_toggle(struct Bitmap * bitmap, size_t index){ 

    bitmap->bits[index / 8] = bitmap->bits[index / 8] ^ ( 0x01 << (7 - (index % 8) ) ); /* XOR */
    
}

void bitmap_clear_all(struct Bitmap * bitmap){
    for (size_t i = 0; i < bitmap->bytes; i++){
       bitmap->bits[i] = 0x0;
    }
}

void bitmap_set_all(struct Bitmap * bitmap){
    for (size_t i = 0; i < bitmap->bytes; i++){
       bitmap->bits[i] = MASK_255;
    }
}

void bitmap_toggle_all(struct Bitmap * bitmap){
    for (size_t i = 0; i < bitmap->bytes; i++){
       bitmap->bits[i] = ~bitmap->bits[i];
    }
}

#endif