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

#define PERSISTANT_DISK "testPersistant.txt"

#define BLOCK_SIZE 4096
#define FS_SIZE 128
#define BLOCK_COUNT (FS_SIZE/BLOCK_SIZE)
#define ADDRESS_SIZE 8
#define NUM_DIRECT_BLOCKS  ((uint32_t)10)
#define INODES_PER_BLOCK 4 //TODO Should change later

#define INODE_BLOCK_COUNT (BLOCK_COUNT * 0.015) // 1.5% blocks reserved for inodes TODO: Check if it is okay
#define NUM_OF_DATA_BLOCKS ((unsigned int)(BLOCK_COUNT - INODE_BLOCK_COUNT - 1))

typedef uint32_t sType;


#endif