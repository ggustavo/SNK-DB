
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char * argv[]){

    char* path_in = "file_1.txt";
    char* path_out = "file2.mp4";

    int in = open(path_in, O_RDONLY);  
    if (in == -1) {
        perror(path_in);
        return EXIT_FAILURE;
    }
    
    int out = open(path_out, O_CREAT | O_RDWR, 0777); 
    if (out == -1) {
        perror(path_out);
        return EXIT_FAILURE;
    }


    int page_size = 1024;
    int total_read = 0;
    int value_read = 0;
    
    char* buffer = (char*) malloc(sizeof(char) * page_size);

    value_read  = read(in,  buffer,  page_size);
    write(out, buffer, value_read);
    total_read += value_read;
    printf("\n  Read: %d ", value_read);
    
    while(value_read > 0){
        //free(buffer);
        //buffer =  (char*) malloc(sizeof(char) * page_size);

        value_read  = read(in,  buffer,  page_size);
        //write(out, buffer, value_read);
        total_read += value_read;
        printf("\n  Read: %d ", value_read);
    }

    printf("\n Total Read: %d ",total_read);

    close(in);
    close(out);
    return EXIT_SUCCESS;
}

