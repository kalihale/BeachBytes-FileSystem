# BeachBytes CS270 Project


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
