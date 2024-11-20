#include "../header/data_block_operation.h"
#include "../header/layerZero.h"

sType allocate_data_block()
{
    if(fs_superblock->freelist_head == 0 && fs_superblock->maxAlloc==NUM_OF_DATA_BLOCKS)
    {
        //need to report error
        //no block left
        return -1;
    }
    sType allocated_bk_no = 0;
    //allocating the first non allocated block
    if(fs_superblock->maxAlloc!=NUM_OF_DATA_BLOCKS){
        allocated_bk_no=fs_superblock->maxAlloc++;
        if(!fs_write_superblock())
        {
            return -1;
        }
    }
    else{
        allocated_bk_no=fs_superblock->freelist_head;
        char buffer[BLOCK_SIZE];
        if(!fs_read_block(fs_superblock->freelist_head, buffer))
        {
            //need to report error
            return false;
        }
        sType nextAlloct= *(sType*)buffer;
        fs_superblock->freelist_head=nextAlloct;
        if(!fs_write_superblock())
        {
            return -1;
        }
    }
    if(allocated_bk_no==0){
        //need to report error
        //no block left
        return -1;
    }
    return allocated_bk_no;
}

char* read_data_block(sType index)
{
    if(index <= INODE_BLOCK_COUNT || index > BLOCK_COUNT)
    {
        //need to report error
        return NULL;
    }

    char* buffer = (char*)malloc(BLOCK_SIZE);
    if(fs_read_block(index, buffer))
    {
        return buffer;
    }
    return NULL;
}

bool write_data_block(sType index, char* buffer)
{
    if(index <= INODE_BLOCK_COUNT || index > BLOCK_COUNT)
    {
       //need to report error
        return false;
    }
    return fs_write_block(index, buffer);
}

bool free_data_block(sType index) {
    if(index <= INODE_BLOCK_COUNT || index > BLOCK_COUNT)
    {
        //need to report error
        return false;
    }

    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    
    //free the last allocated block
    if(index==fs_superblock->maxAlloc-1){
        fs_superblock->maxAlloc-=1;
        if(!fs_write_superblock())
        {
            //need to report error
            return false;
        }
        return true;
    }
    *(sType*)buffer=fs_superblock->freelist_head;
    fs_superblock->freelist_head=index;

    if(!fs_write_superblock()) {
        //need to report error
        return false;
    }

    if(!fs_write_block(index, buffer)) {
        
        return false;
    }
    return true;
}
