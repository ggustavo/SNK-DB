#ifndef EXPORT_JSON_H_INCLUDED
#define EXPORT_JSON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

void export_json_final_result(char *policy_name, int buffer_size, int write, int hits, double time ){
   
    FILE *f;
    f = fopen("result.json","at");
    fprintf(f,"{\"name_p\":\"%s\",\"buffer_size\":%d,\"hits\":%d,\"writes\":%d,\"time\":%f},\n",
            policy_name,buffer_size,hits,write,time);
    fclose(f);

}

#endif
