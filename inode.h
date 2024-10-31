//
// Created by kali on 10/28/24.
//

#ifndef INODE_H
#define INODE_H
#include <cstdint>

const BLOCKSIZE = 4096;

struct inodeStruct {
    // Actual length of all properties (including infoLength and maxPropLen)
    uint32_t infoLength = 912;
    // Device containing the file
    uint32_t deviceID = 0;
    // File Serial Numbers
    uint32_t fileSerialNum = 0;
    char ownerID[32] = "none";
    // groupID of the file
    char groupID[32] = "none";
    // Permissions - TODO AT THE END
    int8_t ownerPermission = 7;
    int8_t groupPermission = 7;
    int8_t worldPermission = 7;
    int32_t linkCount = 0;
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
    uint32_t directAddresses[(BLOCKSIZE - infoLength) / sizeof(uint32_t)];
};



#endif //INODE_H
