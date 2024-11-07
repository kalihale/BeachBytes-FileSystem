#include "fileStructure.h"
#include "common_config.h"
#include "layerZero.h"

bool inodeNum_valid(sType inodeNum){
    return inodeNum >= 0 && inodeNum < INODE_BLOCK_COUNT;
}

void gen_block_offset(sType inodeNum, sType *block, sType *offset){
    *block = (inodeNum / INODES_PER_BLOCK) + 1; //+1 for super block;
    *offset = inodeNum % INODES_PER_BLOCK;
}

inodeStruct* load_iNode_From_Disk(sType inodeNum){
    if(!inodeNum_valid(inodeNum)){
        return NULL;
    }

    sType block; 
    sType offset;

    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];

    if(!fs_read_block(block, buffer)){
        //How to throw an error in fuse
        return NULL;
    }

    inodeStruct* node= (inodeStruct*)buffer;
    node = node + offset;

    inodeStruct* iNode = (inodeStruct*)malloc(sizeof(inodeStruct));
    memcpy(iNode, node, sizeof(inodeStruct));
    node = NULL;
    return iNode;

}

bool writeINodeToDisk(sType inodeNum, inodeStruct* inode){
    if(!inodeNum_valid(inodeNum)){
        return false;
    }

    sType block; 
    sType offset;
    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];
    if(!fs_read_block(block, buffer)){
        //How to log an error in fuse
        return false;
    }

    inodeStruct* node= (inodeStruct*)buffer;
    node = node + offset;

    memcpy(node, inode, sizeof(inodeStruct));

    if(!fs_write_block(block, buffer)){
        //How to log an error in fuse
        return false;
    }

    return true;

}

sType getNextFreeINode(){
    sType curBlock = 0;
    sType curOffset = 0;
    char buffer[BLOCK_SIZE];
    inodeStruct* blockNode;
    inodeStruct* curNode;

    while(curBlock< INODE_BLOCK_COUNT){
        if(!fs_read_block(curBlock, buffer)){
        //How to throw an error in fuse
            return -1;
        }
        blockNode = (inodeStruct*) buffer;
        while(curOffset < INODES_PER_BLOCK){
            curNode = blockNode + curOffset;
            if(!curNode->is_allocated){
                return curBlock*INODES_PER_BLOCK + curOffset;
            }
            curOffset++;

        }
        curOffset = 0;
        curBlock += 1;
    }

    return -1;
}

sType createInode(){
    sType inodeNum = getNextFreeINode();

    if(!inodeNum_valid(inodeNum)){
        return -1;
    }

    sType block;
    sType offset;
    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];

    if(!fs_read_block(block, buffer)){
        //How to throw an error in fuse
        return -1;
    }

    inodeStruct* node = (inodeStruct*) buffer;
    node = node + offset;

    node -> linkCount = 0;
    node -> is_allocated = true;

    if(!fs_write_block(block, buffer)){
        //How to throw an error in fuse
        return -1;
    }

    return inodeNum;

}


bool free_block(inodeStruct* iNode){
    for(sType i = 0; i< NUM_DIRECT_BLOCKS; i++){
        sType block = iNode->directAddresses[i];
        if(block != 0){
            if(!free_data_block(block)){
                return false;
            }
        }
        else{
            //if the direct block is empty, then the indirect ones will also be empty
            //Can we assume this?
            break;
        }
    }

    return true;
    //TODO free indirect blocks
}

bool delete_inode(sType inodeNum){
    if(!inodeNum_valid(inodeNum)){
        return false;
    }

    sType block; 
    sType offset;
    gen_block_offset(inodeNum, &block, &offset);

    char buffer[BLOCK_SIZE];
    if(!fs_read_block(block, buffer)){
        //How to log an error in fuse
        return false;
    }

    inodeStruct* node= (inodeStruct*)buffer;
    node = node + offset;

    //TODO we have to add every allocated block back to the free list. 

    node->deviceID = 0;
    node->fileSerialNum = 0;
    node->ownerPermission = 7;
    node->groupPermission=7;
    node->worldPermission=7;
    node->linkCount=0;
    node->fileSize=0;
    node->blocks=0;
    node->ctime=0;
    node->mtime=0;
    node->atime=0;
    for(sType i = 0; i < NUM_DIRECT_BLOCKS; i++)
        node->directAddresses[i] = 0;
    node->singleIndirect=0;
    node->doubleIndirect=0;
    node->tripleIndirect=0;
    node->is_allocated=false;

    if(!fs_write_block(block, buffer)){
        return false;
    }

    return true;
}