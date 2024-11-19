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

typedef int64_t sType;

#define BLOCK_SIZE 4096
#define FS_SIZE ((uint64_t) 104857600*5)//1024
#define BLOCK_COUNT ((sType)(FS_SIZE/BLOCK_SIZE))
#define ADDRESS_SIZE 8
#define NUM_DIRECT_BLOCKS  ((uint32_t)10)
#define DEFAULT_PERMISSIONS (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

//directory info
#define FILE_NAME_MAX_LENGTH ((sType) 25)
#define RECORD_LENGTH ((unsigned short) 2) // size to stores the info of length
#define RECORD_INUM ((sType) 8)   // size to store the inode
#define RECORD_FIXED_LEN ((unsigned short)(RECORD_LENGTH + RECORD_INUM))

#endif