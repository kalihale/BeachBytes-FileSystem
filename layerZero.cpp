#include <stdarg.h>
#include <unistd.h>

#include "layerZero.h"
#include "fileStructure.h"

bool fs_open()
{
    fs_ptr = open(PERSISTANT_DISK, O_RDWR);
    if (fs_ptr == -1)
    {
        printf(" Error opening device %s\n", PERSISTANT_DISK);
        return false;
    }
    return true;
}

bool fs_close()
{
    if (close(fs_ptr) != 0)
    {
        printf("error while closeing the file\n");
        return false;
    }
    
    return true;
}

bool isBlockOutOfRange(sType blockid)
{
    return (blockid < 0 || blockid >= BLOCK_COUNT);
}

bool fs_read_block(sType blockid, char *buffer)
{
    if (!buffer)
        return false;

    if (isBlockOutOfRange(blockid))
    {
        printf(" !!!!!!!!!! Block id out of range: %u !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", blockid);
        return false;
    }

    sType offset = (unsigned long) BLOCK_SIZE * blockid;
    sType lseek_status = lseek(fs_ptr, offset, SEEK_SET);
    if(lseek_status == -1)
    {
        printf(" reading Falied, lseek to offset %u failed for block id %u\n", offset, blockid);
        return false;
    }
    if(read(fs_ptr, buffer, BLOCK_SIZE) != BLOCK_SIZE)
    {
        printf(" reading Falied, Reading contents of disk failed\n");
        return false;
    }
    return true;
}

bool fs_write_block(sType blockid, char *buffer)
{
    if (!buffer)
        return false;
    if (isBlockOutOfRange(blockid))
    {
         printf(" !!!!!!!!!! Block ID is greater then number allowed blocks: %u !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", blockid);
        return false;
    }

    off_t offset = (unsigned long) BLOCK_SIZE * blockid;
    off_t lseek_status = lseek(fs_ptr, offset, SEEK_SET);
    if(lseek_status == -1)
    {
        printf(" writing Falied, lseek to offset %lu failed for block id %u\n", offset, blockid);
        return false;
    }
    if(write(fs_ptr, buffer, BLOCK_SIZE) != BLOCK_SIZE)
    {
        printf(" writing Falied, : Writing contents to disk failed\n");
        return false;
    }

    return true;
}

bool fs_create_superblock(){
    //TODO

    fs_superblock = (struct superblock*)malloc(sizeof(struct superblock));
    //TODO assign the values to superBlocks

    return fs_write_superblock();
}

bool fs_write_superblock()
{
    if(!fs_superblock){
        printf("SuperBlock Ptr is not initialized\n");
        return false;
    }
    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    memcpy(buffer, fs_superblock, sizeof(struct superblock));
    if(!fs_write_block(0, buffer))
    {
        printf("Error writing superblock to memory.\n");
        return false;
    }
    printf("Superblock written!!!\n");
    return true;
}


bool fs_create_ilist(){
    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    
    // initialize ilist for all blocks meant for inodes
    // start with index = 1 since superblock will take block 0
    for(ssize_t blocknum = 1; blocknum <= INODE_BLOCK_COUNT; blocknum++)
    {
        //fuse_log(FUSE_LOG_DEBUG, "%s Writing blocknum %ld and buffer %s\n",ALTFS_CREATE_ILIST, blocknum, *buffer);
        if (!fs_write_block(blocknum, buffer)){
            printf("Error writing to inode block number in create function\n");
            return false;
        }
    }
    return true;
}
bool fs_init()
{
    bool createSuperBlock = fs_create_superblock();
    if (!createSuperBlock)
    {
        printf("Error creating superblock while initializing FS\n");
        return false;
    }
    printf("Successfully created superblock\n");

    bool createInodeList = fs_create_ilist();
    if (!createInodeList)
    {
        printf("Error creating inode blocks while initializing FS\n");
        return false;
    }
    printf("Successfully created inode blocks\n");

    return true;
}

bool free_data_block(sType index){
    if(index <= INODE_BLOCK_COUNT || index > BLOCK_COUNT){
        return false;
    }

    if(fs_superblock->freelist_head == 0){
        fs_superblock->freelist_head = index;
        if(!fs_write_superblock()){
            return false;
        }
        return true;
    }

    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);

    //Write the value of the current free list head to the data block passed in 
    sType value = fs_superblock->freelist_head;
    memcpy(buffer, &value, sizeof(sType));
    if(!fs_write_block(index, buffer)){
        return false;
    }

    //update the value of the freelist_head and persist it
    fs_superblock->freelist_head = index;
    if(!fs_write_superblock()){
        return false;
    }

    return true;

}