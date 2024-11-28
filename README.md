# BeachBytes
CS270 Project



## To Compile in release mode

```
mkdir build
cd build
touch testPersistant.txt
cmake -DCMAKE_BUILD_TYPE=release ..
make
./setUpDisk
./cs270 <mountFolder>
```


## To Compile in debug mode

```
mkdir build
cd build
touch testPersistant.txt
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./setUpDisk
./cs270 <mountFolder>
```



### Note if using CLion:

If `#include`s aren't working, go to tools>resync remote hosts
