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
|`-DLRU`   |Least Recently Used (LRU)         |`gcc (...) -DLRU` |
|`-DMRU`   |Most Recently Used (MRU)          |`gcc (...) -DMRU` |
|`-DFIFO`  |First In, First Out (FIFO)        |`gcc (...) -DFIFO` |
|`-DLFU`   |Least Frequently Used (LFU)       |`gcc (...) -DLFU` |
|`-DLFUDA` |Least Frequently Used with Dynamic Aging (LFU-DA)       |`gcc (...) -DLFUDA` |
|`-DCLOCK` |Second Chance (or Clock)          |`gcc (...) -DCLOCK` |
|`-DGCLOCK`|Generalized Clock (GCLOCK)        |`gcc (...) -DGCLOCK` |
|`-DMQ`    |Multi queue (MQ)                  |`gcc (...) -DMQ` |
|`-DARC`   |Adaptive Replacement Cache (ARC)*  |`gcc (...) -DARC` |
|`-DFBR`   |Frequency-Based Replacement (FBR)*  |`gcc (...) -DFBR` |
|`-DLRUMIS`   |LRU with Midpoint Insertion Strategy (LRU-MIS) |`gcc (...) -DLRUMIS` |
|`-DF2Q`   |Full Version “Two Queue” (2Q)*  |`gcc (...) -DF2Q` |
|`-DLRUK`   |Least kth-to-last Reference (LRU-K)*  |`gcc (...) -DLRUK` |
|`-DLIRS`   |Low Inter-reference Recency Set (LIRS)*  |`gcc (...) -DLIRS` |
|`-DCFLRU` |Clean-First LRU (CFLRU)*  |`gcc (...) -DCFLRU` |
|`-DLRUWSR` |LRU Write Sequence Reordering (LRU-WSR)*  |`gcc (...) -DLRUWSR` |
|`-DCCFLRU` |Cold-Clean-First LRU (CCF-LRU)*  |`gcc (...) -DCCFLRU` |
|`-DCCCFLRU` |Controllable Cold Clean First Least Recently Used (CCCF-LRU)*  |`gcc (...) -DCCCFLRU` |
|`-DCFDC` |Clean-First Dirty-Clustered (CFDC)*  |`gcc (...) -DCFDC` |
|`-DCASA` |Cost-Aware Self-Adaptive (CASA)*  |`gcc (...) -DCASA` |
|`-DADLRU` |Adaptive Double LRU (AD-LRU)* |`gcc (...) -DADLRU` |
|`-DLLRU` |Locality-aware Least Recently Used (LLRU)* |`gcc (...) -DLLRU` |
|`-DAMLRU` | AM-LRU* |`gcc (...) -DAMLRU` |
|`-DGASA` | Ghost buffer Assisted and Self-tuning Algorithm (GASA)* |`gcc (...) -DGASA` |
|`-SCMBP` | SCMBP-SCCW* |`gcc (...) -SCMBP` |

> **Note:** Use [db_kernel.h](https://github.com/ggustavo/SNK-DB/blob/master/src/dbms/db_kernel.h). If you don't want to use these options

> \* ***in test phase***