//
// Created by kali on 10/28/24.
//

#ifndef INODE_H
#define INODE_H
#include <cstdint>
#include "common_config.h"
#include "layerZero.h"


bool inodeNum_valid(sType inodeNum);

void gen_block_offset(sType inodeNum, sType *block, sType *offset);

struct inodeStruct* load_iNode_From_Disk(sType inodeNum);

bool writeINodeToDisk(sType inodeNum, inodeStruct* inode);

sType getNextFreeINode();

sType createInode();

bool free_indirect_block(sType blockNum, sType recurse);

bool free_block(inodeStruct* iNode);

bool delete_inode(sType inodeNum);

#endif //INODE_H