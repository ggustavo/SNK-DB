#!/bin/sh

WINDOWS_PARAM="-DWIN"
#WINDOWS_PARAM=""
TEST_FILE="src/tests/test_buffer_requests.c"
OUTPUT_FILE="database"
#----------------------------------------------------------------

for entry in "src/dbms/buffer_manager/policies"/*
do

    result=${entry//"src/dbms/buffer_manager/policies"/""}
    result=${result//"/"/"-D"}
    result=${result//".h"/""}
    POLICY=$result
    echo $POLICY   

    gcc $TEST_FILE -o $OUTPUT_FILE $WINDOWS_PARAM $POLICY
    ./$OUTPUT_FILE

done

