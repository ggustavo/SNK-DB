#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "../dbms/db_kernel.h"
#include "../dbms/file_manager/data_file.h"
#include "../dbms/db_export_json.h"
#include <pthread.h>
#include <unistd.h>

struct DataFile * warehouse;
struct DataFile * district;
struct DataFile * customer;
struct DataFile * history;
struct DataFile * new_order;
struct DataFile * order;
struct DataFile * order_line;
struct DataFile * stock;

struct DataFile * map_file_id(int file_id){
       if(file_id == 11) return warehouse;
       if(file_id == 22) return district;
       if(file_id == 33) return customer;
       if(file_id == 44) return history;
       if(file_id == 55) return new_order;
       if(file_id == 66) return order;
       if(file_id == 77) return order_line;
       if(file_id == 88) return stock;
       return NULL;
}

char * workload_test;
char * result_file_test;
char * result_file_test_scan;

void * buffer(void * arg){
    struct timeval start_time, end_time;
    char operation;
	int data_file_id = 11;
    int block_id;
    FILE * file = fopen(workload_test, "r");

    if (file == NULL) {
		printf("\n workload not found \n");
        return NULL;
	}    

    int max_block_id = 0;

    FILE * scans_results;
    scans_results = fopen(result_file_test_scan,"at");
    fprintf(scans_results,"%s","\n");
    
    int cc_ = 0;

	gettimeofday(&start_time, NULL); // Start Time
    while (fscanf(file, "%c;%d\n", &operation, &block_id) > 0) {
        cc_ ++;
        if(block_id > max_block_id) max_block_id = block_id;
       
        //printf("\nRequest ---> %c[%d-%d]", operation, map_file_id(data_file_id)->file_id, block_id);		
        
        if(operation == 'R')buffer_request_page( map_file_id(data_file_id)->file_id, block_id, READ_REQUEST);
        if(operation == 'W')buffer_request_page( map_file_id(data_file_id)->file_id, block_id, WRITE_REQUEST);
        //buffer_print_policy();

        export_csv_current_state(scans_results, BUFFER_POLICY_NAME,  flush_operations, hit_operations, operation, block_id, cc_);
    
    }  

    
    gettimeofday(&end_time, NULL); // End Time

    //buffer_flush(); 
	buffer_print_statistics();
    // Show Execution Time in Seconds
    double time =  ( ((double) ((double)end_time.tv_usec - (double)start_time.tv_usec)) / 1000000 ) +  ( (double) ((double)end_time.tv_sec - (double)start_time.tv_sec) )  ;
    
    printf("\nExecution Time = %f seconds\n", time);
    printf("\nMax Block ID = %d\n", max_block_id);

    export_json_final_result(result_file_test ,BUFFER_POLICY_NAME, BUFFER_SIZE, flush_operations, hit_operations, time);
	
    fprintf(scans_results,"%s","\n");
    fclose(scans_results);
    //printf("\nPress Any Key to Exit\n");
	//getchar();
    return NULL;
}

int ANALYZE_THREAD_FLAG = 1;

void * analyze_thread(void * arg){
    int iternal = 0;
    #ifdef ML3 
    while (ANALYZE_THREAD_FLAG == 1) {
        //printf("\nAnalyze Thread ...");
            //if(GC > iternal + 50){
                analyze(NULL); 
                //iternal = GC; 
           // }
        //sleep(0.0001);
    }
    #endif

    #ifdef ML4
    while (ANALYZE_THREAD_FLAG == 1) {
        //printf("\nAnalyze Thread ...");
            //if(GC > iternal + 50){
                analyze(NULL); 
                //iternal = GC; 
           // }
        //sleep(0.0001);
    }
    #endif
    return NULL;
}

int main(int argc, char *argv[]) {
    
    if (argc <= 1 ){
        printf("[ERR0] Missing parameters\n");
        return 1;
    }

    char * result_file = catalog_append_path(2, argv[1], ".js");
    printf("\n path: %s\n", argv[2]);

	start_database();
    
    //------------------ Initializing data files -----------------
    warehouse  = data_file_open("warehouse",  CATALOG_DATA_FOLDER);
    district   = data_file_open("district",   CATALOG_DATA_FOLDER);
    customer   = data_file_open("customer",   CATALOG_DATA_FOLDER);
    history    = data_file_open("history",    CATALOG_DATA_FOLDER);
    new_order  = data_file_open("new_order",  CATALOG_DATA_FOLDER);
    order      = data_file_open("order",      CATALOG_DATA_FOLDER);
    order_line = data_file_open("order_line", CATALOG_DATA_FOLDER);
    stock      = data_file_open("stock",      CATALOG_DATA_FOLDER);
    //-------------------------------------------------------------
    printf("\n---------------------------------------------------------------\n");
    workload_test = argv[2];
    result_file_test = result_file;


    char str[20];
    sprintf(str, "scan_%d.csv", BUFFER_SIZE);

    result_file_test_scan = str;

    pthread_t workload_th;
    pthread_t analyze_th;
    
    pthread_create(&workload_th, NULL, buffer, NULL);
    pthread_create(&analyze_th, NULL, analyze_thread, NULL);

    pthread_join(workload_th, NULL);
    ANALYZE_THREAD_FLAG = 0;
    pthread_join(analyze_th, NULL);
	return 0;
}