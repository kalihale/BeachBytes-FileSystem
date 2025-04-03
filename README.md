# BeachBytes FileSystem Project


## To Compile in release mode

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=release ..
make
./setUpDisk
./cs270 <mountFolder>
```


## To Compile in debug mode

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./setUpDisk
./cs270 <mountFolder>
```

## Resources consulted
https://libfuse.github.io/doxygen/

https://github.com/libfuse/libfuse

https://www.unix.com/man-page-collection.php?code=301&os=linux
