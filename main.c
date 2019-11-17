
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "file.h"


char* cypher_buffer(int block_size, char* buffer){

    //
    //:TODO: 
    //

    return buffer;
}

int main(int argc, char * argv[]){



    char* path_in = "file_1.mp4";
    char* path_out = "result.mp4";

    FILE * in = open_file(path_in);
    FILE * out = fopen(path_out, "wb");

    int block_size = 1024 * 8 * 10;
    int total_read = 0;
    int value_read = 0;
    int size = file_size(in);

    char* buffer = (char*) malloc(sizeof(char) * block_size);

    int n = size/block_size;

    for(int i = 0; i < n; i++ ){

        free(buffer);
        buffer = (char*) malloc(sizeof(char) * block_size);

        fseek(in, i * block_size, SEEK_SET);
	    value_read = fread(buffer, sizeof(char), block_size, in);
        total_read += value_read;
        
        fseek(out, i * block_size, SEEK_SET);
	    fwrite(buffer, sizeof(char), block_size ,out);
	    
     //   printf("\n read: %d p-> %d",value_read, ftell(in));
    }

    int rest = size % block_size;
    if(rest > 0){
        free(buffer);
        buffer = (char*) malloc(sizeof(char) * rest);
        
        fseek(in, size - rest, SEEK_SET);
	    value_read = fread(buffer, sizeof(char), rest, in);
        total_read += value_read;
        
        fseek(out, size - rest, SEEK_SET);
	    fwrite(buffer, sizeof(char), rest ,out);
    }

    fflush(out);
    
    printf("\n Block size: %d ",block_size); 
    printf("\n Rest: %d ", rest); 
    printf("\n Total Interactions: %d ",n); 
    printf("\n Total Read: %d/%d ",total_read,size);

    fclose(in);
    fclose(out);
    return EXIT_SUCCESS;
}

