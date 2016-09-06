##PSI##
###Cuckoo Hashing###
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
---
###Dependencies:
---
libglib2.0-dev, _lpsi-util_, libssl-dev

###Usage:
```
psi-simple-hashing  -1 16-Byte seed 1 -2 16-Byte seed 2  -3 16-Byte seed 3 -p path to data  -s path buckets -b bucket number -q queue buffer size -i read buffer size -t thread number -d table size multiplier -f fixed table size -z path result
```
