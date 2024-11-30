# BeachBytes CS270 Project

## To set up the loop device

1. Create a block file of the correct size (bs=block size, count=number of blocks).
```
dd if=/dev/zero of=myblockfile bs=1M count=100
```
2. Associate the block file with a loop device.
```
sudo losetup /dev/loop4 myblockfile
```
    2a. If the block loop device is in use, use `losetup -f` to find the first one not in use.
3. Change permissions to the loop device with chmod.
```
sudo chmod 666 /dev/loop4
```
4. Run as below.

## To remove the loop device

```
sudo umount <mount directory>
sudo losetup -d /dev/loop4
```


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
