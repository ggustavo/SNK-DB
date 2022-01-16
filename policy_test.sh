#!/bin/sh

WINDOWS_PARAM="-DWIN"
#WINDOWS_PARAM=""
TEST_FILE="src/tests/test_buffer_workload.c"
OUTPUT_FILE="database"
#----------------------------------------------------------------
POLICY="-DML1"

for buffer_size in 1000 2000 4000 8000 16000 32000
do
    gcc $TEST_FILE -o $OUTPUT_FILE $WINDOWS_PARAM $POLICY -DBUFFER_SIZE=$buffer_size
    ./$OUTPUT_FILE
done





