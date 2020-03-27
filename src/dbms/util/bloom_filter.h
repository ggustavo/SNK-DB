/*
 * bloom filter implementation 
 */

#ifndef BLOOM_FILTER
#define BLOOM_FILTER
#include "bitmap.h"
#include "hash_functions.h"

struct BloomFilter{
    size_t size;
    struct Bitmap * bitmap1;
    struct Bitmap * bitmap2;
    struct Bitmap * bitmap3;
};

struct BloomFilter * bloom_filter_create(size_t size){
    
    struct BloomFilter * bloom_filter = (struct BloomFilter * ) malloc(sizeof(struct BloomFilter));
    bloom_filter->size = size;

    bloom_filter->bitmap1 = bitmap_create(size);
    bitmap_clear_all(bloom_filter->bitmap1);
   
    bloom_filter->bitmap2 = bitmap_create(size);
    bitmap_clear_all(bloom_filter->bitmap2);
   
    bloom_filter->bitmap3 = bitmap_create(size);
    bitmap_clear_all(bloom_filter->bitmap3);

    return bloom_filter;
}

void bloom_filter_set(struct BloomFilter * bloom_filter, char* value, int size){
    unsigned int key1 = hash_djb2(value, size)    % bloom_filter->bitmap1->size;
    unsigned int key2 = hash_jenkins(value, size) % bloom_filter->bitmap2->size;
    unsigned int key3 = hash_sdbm(value, size)    % bloom_filter->bitmap3->size;
    
    bitmap_set(bloom_filter->bitmap1, key1);
    bitmap_set(bloom_filter->bitmap2, key2);
    bitmap_set(bloom_filter->bitmap3, key3);
}

int bloom_filter_get(struct BloomFilter * bloom_filter, char* value, int size){
    unsigned int key1 = hash_djb2(value, size)    % bloom_filter->bitmap1->size;
    unsigned int key2 = hash_jenkins(value, size) % bloom_filter->bitmap2->size;
    unsigned int key3 = hash_sdbm(value, size)    % bloom_filter->bitmap3->size;

    return bitmap_get(bloom_filter->bitmap1,key1) * 
           bitmap_get(bloom_filter->bitmap2,key2) *
           bitmap_get(bloom_filter->bitmap3,key3);

}

#endif