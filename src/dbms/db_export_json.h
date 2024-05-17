#ifndef EXPORT_JSON_H_INCLUDED
#define EXPORT_JSON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

void export_json_final_result(char * worload, char *policy_name, int buffer_size, int write, int hits, double time ){
   
    FILE *f;
    f = fopen(worload,"at");
    fprintf(f,"{\"name_p\":\"%s\",\"buffer_size\":%d,\"hits\":%d,\"writes\":%d,\"time\":%f},\n",
            policy_name,buffer_size,hits,write,time);
    fclose(f);

}

void export_csv_current_state(FILE *f, char *policy_name, int write, int hits, char operation, int id, int r){
    
    /*fprintf(f,"{\"operation\":%c,\"id\":%d,\"name_p\":\"%s\",\"buffer_size\":%d,\"hits\":%d,\"writes\":%d},\n",
            operation,id,policy_name,buffer_size,hits,write);
    */

     fprintf(f,"%d,%s,%d,%c,%d,%d \n",
            r,policy_name,id,operation,hits,write);
    
}

#endif
