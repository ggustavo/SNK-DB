# SNK Database

[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/ggustavo/SNK-DB)  [![Run on Repl.it](https://repl.it/badge/github/replit/crosis)](https://repl.it/github/ggustavo/SNK-DB)

A simple file engine to study techniques to develop database management systems.
[Project Activities](https://github.com/ggustavo/SNK-DB/projects/1?fullscreen=true)


####  Download
Clone the project or [download](https://github.com/ggustavo/SNK-DB/archive/master.zip) directly (zip file)

```shell
git clone https://github.com/ggustavo/SNK-DB.git
```
> **Note:** Strongly recommend run on [gitpod](https://gitpod.io/#https://github.com/ggustavo/SNK-DB) or [repl.it](https://repl.it/github/ggustavo/SNK-DB) platform
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
|`-DLRU`   |Uses Least Recently Used (LRU)         |`gcc (...) -DLRU` |
|`-DMRU`   |Uses Most Recently Used (MRU)          |`gcc (...) -DMRU` |
|`-DFIFO`  |Uses First In, First Out (FIFO)        |`gcc (...) -DFIFO` |
|`-DLFU`   |Uses Least Frequently Used (LFU)       |`gcc (...) -DLFU` |
|`-DLFUDA` |Uses Least Frequently Used with Dynamic Aging (LFU-DA)       |`gcc (...) -DLFUDA` |
|`-DCLOCK` |Uses Second Chance (or Clock)          |`gcc (...) -DCLOCK` |
|`-DGCLOCK`|Uses Generalized Clock (GCLOCK)        |`gcc (...) -DGCLOCK` |
|`-DMQ`    |Uses Multi queue (MQ)                  |`gcc (...) -DMQ` |
|`-DARC`   |Uses Adaptive Replacement Cache (ARC) ***(in test phase)***  |`gcc (...) -DARC` |
|`-DFBR`   |Uses Frequency-Based Replacement (FBR) ***(in test phase)***  |`gcc (...) -DFBR` |
|`-DLRUMIS`   |Uses LRU with Midpoint Insertion Strategy (LRU-MIS) ***(in test phase)***  |`gcc (...) -DLRUMIS` |
|`-DF2Q`   |Uses Full Version “Two Queue” (2Q) ***(in test phase)***  |`gcc (...) -DF2Q` |
|`-DLRUK`   |Uses Least kth-to-last Reference (LRU-K) ***(in test phase)***  |`gcc (...) -DLRUK` |
|`-DLIRS`   |Uses Low Inter-reference Recency Set (LIRS) ***(in test phase)***  |`gcc (...) -DLIRS` |
|`-DCFLRU` |Uses Clean-First LRU (CFLRU) ***(in test phase)***  |`gcc (...) -DCFLRU` |

> **Note:** Use [db_kernel.h](https://github.com/ggustavo/SNK-DB/blob/master/src/dbms/db_kernel.h). If you don't want to use these options


