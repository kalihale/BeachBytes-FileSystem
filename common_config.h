#ifndef __COMMON_CONFIG__
#define __COMMON_CONFIG__

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 35
#endif


#include <fuse.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>


#define PERSISTANT_DISK "testPersistant.txt"

#define BLOCK_SIZE 4096
#define FS_SIZE 409600000
#define BLOCK_COUNT (FS_SIZE/BLOCK_SIZE)
#define ADDRESS_SIZE 4 // if we are using an unsigned int, the address size is 4 bytes

#define NUM_DIRECT_BLOCKS  ((uint32_t)10)
#define INODES_PER_BLOCK BLOCK_SIZE/sizeof(inodeStruct) //TODO Should change later

#define INODE_BLOCK_COUNT ((sType)(BLOCK_COUNT * 0.015)) // 1.5% blocks reserved for inodes TODO: Check if it is okay
#define NUM_OF_DATA_BLOCKS ((unsigned int)(BLOCK_COUNT - INODE_BLOCK_COUNT - 1))

typedef uint32_t sType;

struct superblock
{
    sType freelist_head;
    sType iList_size = INODE_BLOCK_COUNT; //number of blocks in our iList
    sType partitionSize = NUM_OF_DATA_BLOCKS;

};


struct inodeStruct {
    // Actual length of all properties (including infoLength and maxPropLen)
    sType infoLength = 912;
    // Device containing the file
    sType deviceID = 0;
    // File Serial Numbers
    sType fileSerialNum = 0;
    char ownerID[32] = "none";
    // groupID of the file
    char groupID[32] = "none";
    // Permissions - TODO AT THE END
    int8_t ownerPermission = 7;
    int8_t groupPermission = 7;
    int8_t worldPermission = 7;
    sType linkCount = 0;
    // Size of file in bytes
    int64_t fileSize = 0;
    // number of blocks allocated to this file
    int64_t blocks = 0;
    // inode change time
    int32_t ctime = 0;
    // file content modification time
    int32_t mtime = 0;
    // file access time
    int32_t atime = 0;
    //direct block
    sType directAddresses[NUM_DIRECT_BLOCKS];
    //single indirect block num
    sType singleIndirect; 
    //double indirect block num
    sType doubleIndirect;
    //triple indirect block num
    sType tripleIndirect;
    //is iNode allocated
    bool is_allocated = false;

};


#endif