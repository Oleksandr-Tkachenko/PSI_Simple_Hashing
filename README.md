##PSI##
###Cuckoo Hashing###

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

__Dependencies:__ libglib2.0-dev, _lpsi-util_, libssl-dev

__Usage:__ 
```
_psi-simple-hashing_  -1 _16-Byte seed 1_ -2 _16-Byte seed 2_  -3 _16-Byte seed 3_ -p _"path to data"_  -s _"path buckets"_ -b _bucket number_ -q _queue buffer size_ -i _read buffer size_ -t _threads_ -d _table size multiplier_ -f _fixed table size_ -z _path_result_
```
