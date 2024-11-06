//
// Created by kali on 10/28/24.
//

#ifndef INODE_H
#define INODE_H
#include <cstdint>
#include "common_config.h"



struct inodeStruct* loadINodeFromDisk(sType inodeNum);

bool writeINodeToDisk(sType inodeNum, inodeStruct* inode);

sType createInode();

bool delete_inode(sType inodeNum);

#endif //INODE_H