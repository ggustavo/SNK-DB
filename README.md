
# SNK Database

A simple file engine to study techniques to develop database management systems.
[Project Activities](https://github.com/ggustavo/SNK-DB/projects/1?fullscreen=true)



## Compiling

#### Use GCC to compile

```properties
cd SNK-DB
gcc src/tests/test_data_file.c -o database -Wall -Wextra 
```  
> **Note:** Use -Wall and -Wextra to show warnings 

#### Running 
```properties
./database
```  
#### General Extra Options

|Option          |Action               |Example		 |
|----------------|---------------------|------------ |
|`-DBLOCK_SIZE`  |sets block size      |`gcc (...) -DBLOCK_SIZE=8192` |
|`-DBUFFER_SIZE` |sets buffer size     |`gcc (...) -DBUFFER_SIZE=100000` |                 

> **Note:** Use [db_config.h](https://github.com/ggustavo/SNK-DB/blob/master/src/dbms/db_config.h). If you don't want to use these options

#### Buffer Replacement Policies Extra Options

|Name          |Action                           |Example		   |
|----------------|-------------------------------|------------     |
|`-DLRU`  |uses Least Recently Used (LRU)        |`gcc (...) -DLRU` |
|`-DMRU`  |uses Most Recently Used (LRU)         |`gcc (...) -DMRU` |
|`-DFIFO` |uses First In, First Out (FIFO)       |`gcc (...) -DFIFO` |

> **Note:** Use [db_kernel.h](https://github.com/ggustavo/SNK-DB/blob/master/src/dbms/db_kernel.h). If you don't want to use these options


