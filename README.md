# Private Set Intersection (PSI)
###Simple Hashing###

A memory-efficient implementation of the Simple Hashing algorithm.
Takes 16-byte elements as input from a file. Writes output to a separate file.

Buckets are files where the elements are temporary stored. Memory queues are
 built for every bucket, so that we don't have to write down every element after
every read operation. Elements are read from input file sequentially and saved 
in the queue buffer according to its hash value. If queue buffer is full it will
 be written to a physical entity of the bucket(file). After all elements are
 read from the input file all remaining bucket queues are going to be dumped to
 their files.

Because of the sorted order of buckets, we now can build simple hashing table 
using only one bucket at the time, so that it fits in RAM.

---
__Install:__
```
sudo make install
```
__Clean:__ 
```
sudo make clean
```
__Remove:__ 
```
sudo make remove
```

###Dependencies:
---
 * libglib2.0-dev 
 * lpsi-util
 * libssl-dev

###Usage:
---
__Usage:__ 
`psi-simple-hashing`
* -1 _16-Byte seed 1_ Hash seed
* -2 _16-Byte seed 2_ Hash seed
* -3 _16-Byte seed 3_ Hash seed
* -p _"path to data"_ Path to the input raw data
* -s _"path to buckets"_ Folder where to place buckets
* -b _number of buckets_ 
* -q _queue buffer size_ Buffer size for bucket queue
* -i _read buffer size_ Buffer size of chunk to be read from input file
* -t _threads_ Number of used threads
* -d _table size multiplier_ Size of hash table is calculated by multiplying 
number of elements by set value
* -f _fixed table size_ Fixed table size value. Stronger than table size multiplier
* -z _path_result_ Path to the output file
* -r _1_ Reduction of elements 16 to 10 bytes. Bin flag is not removed, so +1 byte.


http://encrypto.de
