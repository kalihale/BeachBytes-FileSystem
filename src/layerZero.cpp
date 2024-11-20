#include <stdarg.h>
#include <unistd.h>

#include "../header/layerZero.h"
#include "../header/common_config.h"

struct superblock* fs_superblock = NULL;


bool fs_open()
{
    if(inMemory){
        fs_memory_ptr = (char*)malloc(FS_SIZE);
        memset(fs_memory_ptr, 0, FS_SIZE);
    }
    else{
        fs_ptr = open(PERSISTANT_DISK, O_RDWR);
        if (fs_ptr == -1)
        {
            printf(" Error opening device %s\n", PERSISTANT_DISK);
            return false;
        }
    }
    return true;
}

bool load_FS(){
    if(!fs_open()){
        return false;
    };
    if(fs_superblock == NULL)
    {
        fs_superblock = (struct superblock*)malloc(sizeof(struct superblock));
    }

    char sb_buffer[BLOCK_SIZE];
    memset(sb_buffer, 0, BLOCK_SIZE);
    fs_read_block(0, sb_buffer);

    struct superblock* sb = (struct superblock*)sb_buffer;


    memcpy(fs_superblock, sb, sizeof(struct superblock));

    //fs_read_block(0,(char*) fs_superblock);
    if(fs_superblock->inodes_count == 0)
    {
        printf("failed to read superBlock\n\n");
        //error while loading the superBlock
        return false;
    }
    return true;
}

bool fs_close()
{
    if(inMemory){
        if(fs_memory_ptr != nullptr){
            free(fs_memory_ptr);
        }
        else{
            printf("Attempting to free memory before it was allocated\n");
            return false;
        }
    }
    else{
        if (close(fs_ptr) != 0)
            {
                printf("error while closeing the file\n");
                return false;
            }
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

    if(inMemory){
        sType offset = (sType) BLOCK_SIZE * blockid;
        memcpy(buffer, fs_memory_ptr+offset, BLOCK_SIZE);
    }
    else{
        sType offset = (unsigned long) BLOCK_SIZE * blockid;
        sType lseek_status = lseek(fs_ptr, offset, SEEK_SET);
        if(lseek_status == -1)
        {
            printf("Reading failed, lseek to offset %u failed for block id %u\n", offset, blockid);
            return false;
        }
        if(read(fs_ptr, buffer, BLOCK_SIZE) != BLOCK_SIZE)
        {
            printf("Reading failed, reading contents of disk failed\n");
            return false;
        }
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

    if(inMemory){
        sType offset = (sType) BLOCK_SIZE * blockid;
        memcpy(fs_memory_ptr+offset, buffer,  BLOCK_SIZE);
    }
    else{
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
    }
    

    return true;
}


bool fs_create_superblock(){
    fs_superblock = (struct superblock*)malloc(sizeof(struct superblock));
    fs_superblock->inodes_count = INODE_BLOCK_COUNT * (INODES_PER_BLOCK);

    fs_superblock->iList_size = INODE_BLOCK_COUNT;
    fs_superblock->partitionSize = NUM_OF_DATA_BLOCKS;
    fs_superblock->maxAlloc=INODE_BLOCK_COUNT+1;

    if(fs_superblock->inodes_count == 0)
    {
        printf("failed to read superBlock\n\n");
        //error while loading the superBlock
        return false;
    }
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
        if (!fs_write_block(blocknum, buffer)){
            printf("Error writing to inode block number in create function\n");
            return false;
        }
    }
    return true;
}
bool fs_init()
{
    fs_open();
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
    if(!inMemory){
        fs_close();
    }
    return true;
}

void restartDisk()
{
    if(!fs_write_superblock())
    {
        return;
    }
    
    free(fs_superblock);
    
    if(!fs_close())
    {
        return;
    }
}