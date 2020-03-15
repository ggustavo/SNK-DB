# SNK Database

[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/ggustavo/SNK-DB) 

A simple file engine to study techniques to develop database management systems.
[Project Activities](https://github.com/ggustavo/SNK-DB/projects/1?fullscreen=true)


####  Download
Clone the project or [download](https://github.com/ggustavo/SNK-DB/archive/master.zip) directly (zip file)

```shell
git clone https://github.com/ggustavo/SNK-DB.git
```
> **Note:** I strongly recommend using the gitpod platform [gitpod platform](https://gitpod.io/#https://github.com/ggustavo/SNK-DB)
#### Use GCC to compile

```properties
cd SNK-DB
gcc src/tests/test_buffer_requests.c -o database -Wall -Wextra
```  
> **Note:** Use -Wall and -Wextra to show warnings 

#### Running 
```properties
./database
```  
#### General Extra Options

|Option          |Action               |Example		 |
|----------------|---------------------|------------ |
|`-DBLOCK_SIZE`  |Sets block size      |`gcc (...) -DBLOCK_SIZE=8192` |
|`-DBUFFER_SIZE` |Sets buffer size     |`gcc (...) -DBUFFER_SIZE=100000` |                 

> **Note:** Use [db_config.h](https://github.com/ggustavo/SNK-DB/blob/master/src/dbms/db_config.h). If you don't want to use these options

#### Buffer Replacement Policies Extra Options

|Name          |Action                           |Example		   |
|----------------|-------------------------------|------------     |
|`-DLRU`  |Uses Least Recently Used (LRU)         |`gcc (...) -DLRU` |
|`-DMRU`  |Uses Most Recently Used (MRU)          |`gcc (...) -DMRU` |
|`-DFIFO` |Uses First In, First Out (FIFO)        |`gcc (...) -DFIFO` |
|`-DARC`  |Uses Adaptive Replacement Cache (ARC) ***(in test phase)***  |`gcc (...) -DARC` |

> **Note:** Use [db_kernel.h](https://github.com/ggustavo/SNK-DB/blob/master/src/dbms/db_kernel.h). If you don't want to use these options


