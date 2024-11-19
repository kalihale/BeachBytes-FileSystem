#include "inode.h"

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

    if(!node->is_allocated){
        printf("Inode is not Allocated");
        return NULL;
    }

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
    sType curBlock = 1;
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
                return (curBlock-1)*INODES_PER_BLOCK + curOffset;
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

    inodeStruct* node = (inodeStruct*)(buffer + offset * sizeof(inodeStruct));
    node -> linkCount = 0;
    node -> is_allocated = true;

    if(!fs_write_block(block, buffer)){
        //How to throw an error in fuse
        return -1;
    }

    return inodeNum;

}

//recurse = 1 single indirect
//recurse = 2 double indirect
//recurse = 3 triple indirect
bool free_indirect_block(sType blockNum, sType recurse){
    char buffer[BLOCK_SIZE];
    if(!fs_read_block(blockNum, buffer)){
        return false;
    }
    sType* blocks = (sType*)buffer;
    for(sType i = 0; i< BLOCK_SIZE/ADDRESS_SIZE; i++){
        sType blockid = blocks[i];
        if(blockid == 0){
                break;
        }
        if(recurse == 1){
            //these block numbers correspond to direct blocks
            if(!free_data_block(blockid)){
                return false;
            }
        }
        else{
            //we are freeing a double or triple indirect block, so we need to recurse again.
            if(!free_indirect_block(blockid, recurse-1)){
                return false;
            }
        }
        
    }

    return true;

}


bool free_block(inodeStruct* iNode){
    for(sType i = 0; i< NUM_DIRECT_BLOCKS; i++){
        sType blockid = iNode->directAddresses[i];
        if(blockid != 0){
            if(!free_data_block(blockid)){
                return false;
            }
        }
        else{
            //if the direct block is empty, then the indirect ones will also be empty
            //Can we assume this?
            return true;
        }
    }

    // free single indirect blocks
    if(iNode->singleIndirect != 0){
        if(!free_indirect_block(iNode->singleIndirect, 1)){
            return false;
        }
    }

    if(iNode->doubleIndirect != 0){
        if(!free_indirect_block(iNode->doubleIndirect, 2)){
            return false;
        }
    }

    if(iNode->tripleIndirect != 0){
        if(!free_indirect_block(iNode->tripleIndirect, 3)){
            return false;
        }
    }
    
    return true;
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

    if(!free_block(node)){
        return false;
    }

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