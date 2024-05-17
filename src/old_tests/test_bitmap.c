/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../dbms/util/bitmap.h"

int main(void) {

	struct Bitmap * bitmap = bitmap_create(8 * 7);
    printf("\nbitmap size: %d, bytes: %d", bitmap->size, bitmap->bytes);

    memcpy(bitmap->bits, "gustavo", 7);;

    for (size_t i = 0; i < bitmap->size; i++){
        if(i % 8 == 0)printf("\n");
        printf("%d",bitmap_get(bitmap,i));
    }
    printf("\n\n");
    
    //bitmap_set(bitmap, 0);
    //bitmap_clear_all(bitmap);
    bitmap_toggle_all(bitmap);
 
    for (size_t i = 0; i < bitmap->size; i++){
        if(i % 8 == 0)printf("\n");
        //bitmap_toggle(bitmap,i);
        printf("%d",bitmap_get(bitmap,i));
    }
	return EXIT_SUCCESS;
}

*/