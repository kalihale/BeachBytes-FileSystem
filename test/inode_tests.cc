#include <gtest/gtest.h>
#include "../header/common_config.h"
#include "../header/layerZero.h"
#include "../header/inode.h"

namespace {
class inodeTests: public::testing::Test{
protected:

    void SetUp() override {
        fs_init();
    }

    void TearDown() override {
        fs_close();
    }

};




TEST_F(inodeTests, DeleteInode) {
    sType inodeNum = createInode();
    EXPECT_TRUE(delete_inode(inodeNum)); 
}


TEST_F(inodeTests, InodeNumValid) {
    EXPECT_TRUE(inodeNum_valid(1));
    EXPECT_FALSE(inodeNum_valid(-1));
    EXPECT_FALSE(inodeNum_valid(INODE_BLOCK_COUNT));
}

TEST_F(inodeTests, GenBlockOffset) {
    sType block, offset;
    gen_block_offset(10, &block, &offset);
    EXPECT_EQ(block, (10 / INODES_PER_BLOCK) + 1);  
    EXPECT_EQ(offset, 10 % INODES_PER_BLOCK);      
}


TEST_F(inodeTests, LoadINodeFromDisk) {
    EXPECT_EQ(loadINodeFromDisk(-1), nullptr);
    sType inodeNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeNum);
    ASSERT_NE(inode, nullptr);  
    free(inode); 
} 

TEST_F(inodeTests, WriteINodeToDisk) {
    inodeStruct inode;
    inode.is_allocated = true;
    EXPECT_FALSE(writeINodeToDisk(-1, &inode)); 
    EXPECT_TRUE(writeINodeToDisk(1, &inode));  
}

TEST_F(inodeTests, GetNextFreeINode) {
    sType freeInode = getNextFreeINode();
    EXPECT_GE(freeInode, 0); 
}

TEST_F(inodeTests, CreateInode) {
    sType inodeNum = createInode();
    EXPECT_EQ(inodeNum, 0); 
}


TEST_F(inodeTests, FindCorrectFreeINode) {
    sType inodeNum0 = createInode();
    sType inodeNum1 = createInode();
    sType inodeNum2 = createInode();
    sType inodeNum3 = createInode();
    sType inodeNum4 = createInode();

    EXPECT_EQ(inodeNum0, 0);
    EXPECT_EQ(inodeNum1, 1);
    EXPECT_EQ(inodeNum2, 2);
    EXPECT_EQ(inodeNum3, 3);
    EXPECT_EQ(inodeNum4, 4);

    EXPECT_EQ(5, getNextFreeINode());
}

TEST_F(inodeTests, TestDataPersistence){
    sType inodeNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeNum); 

    inode->tripleIndirect = 5;
    inode->doubleIndirect = 10;
    inode->singleIndirect = 25;
    inode->directAddresses[0] = 1;
    inode->directAddresses[1] = 1;
    inode->atime = 25;
    inode->mtime = 10;
    strncpy(inode->ownerID , "Rithwik", sizeof(inode->ownerID)-1);
    inode->ownerID[sizeof(inode->ownerID) -1] = '\0';
    writeINodeToDisk(inodeNum, inode);

    inodeNum = createInode();
    inode = loadINodeFromDisk(inodeNum); 

    inode = loadINodeFromDisk(0);

    ASSERT_EQ(inode->tripleIndirect , 5) ;
    ASSERT_EQ(inode->doubleIndirect , 10) ;
    ASSERT_EQ(inode->singleIndirect , 25) ;
    ASSERT_EQ(inode->directAddresses[0] , 1) ;
    ASSERT_EQ(inode->directAddresses[1] , 1) ;
    ASSERT_EQ(inode->atime , 25) ;
    ASSERT_EQ(inode->mtime , 10) ;
    ASSERT_EQ(strcmp(inode->ownerID, "Rithwik"), 0);
    free(inode);
}

TEST_F(inodeTests, CreatingInodesAcrossBlocks){
     sType inodeNum;
    for(int i = 0; i< 25; i++){
        inodeNum = createInode();
        ASSERT_EQ(inodeNum, i);
    }
    inodeStruct* inode = loadINodeFromDisk(inodeNum);
    ASSERT_NE(inode, nullptr);  
    free(inode);    
} 

TEST_F(inodeTests, DeleteInodeInMiddle){
    sType inodeNum0 = createInode();
    sType inodeNum1 = createInode();
    sType inodeNum2 = createInode();
    sType inodeNum3 = createInode();
    sType inodeNum4 = createInode();

     EXPECT_TRUE(delete_inode(inodeNum2)); 
     ASSERT_EQ(nullptr, loadINodeFromDisk(inodeNum2));

    inodeNum2 = createInode();
    ASSERT_EQ(inodeNum2, 2);

    EXPECT_TRUE(delete_inode(inodeNum3)); 
    ASSERT_EQ(nullptr, loadINodeFromDisk(inodeNum3));
    inodeNum3 = createInode();
    ASSERT_EQ(inodeNum3, 3);

}

TEST_F(inodeTests, AddDirectBlock) {
    sType inodeNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeNum); 
    sType block_num = allocate_data_block();
    bool result = add_datablock_to_inode(inode, block_num);
    EXPECT_TRUE(result);
    EXPECT_TRUE(inode->blocks == 1);
    EXPECT_TRUE(inode->directAddresses[0] = INODE_BLOCK_COUNT+2);
}

TEST_F(inodeTests, AddSingleIndirectBlock) {
    sType inodeNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeNum); 

    for(int i = 1; i <= NUM_DIRECT_BLOCKS; i++) {
        EXPECT_TRUE(add_datablock_to_inode(inode, INODE_BLOCK_COUNT+1+i));
    }

    bool result = add_datablock_to_inode(inode, INODE_BLOCK_COUNT+1+11);
    EXPECT_TRUE(result);
    EXPECT_TRUE(inode->blocks == NUM_DIRECT_BLOCKS+1);
    EXPECT_TRUE(inode->singleIndirect = INODE_BLOCK_COUNT+1+11);
}

TEST_F(inodeTests, AddDoubleIndirectBlock) {
    sType inodeNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeNum); 
    sType totalBlocks = NUM_DIRECT_BLOCKS+NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;
    for(int i = 1; i <= totalBlocks; i++) {
        EXPECT_TRUE(add_datablock_to_inode(inode, INODE_BLOCK_COUNT+1+i));
    }

    bool result = add_datablock_to_inode(inode, INODE_BLOCK_COUNT+totalBlocks+1);
    EXPECT_TRUE(result);
    EXPECT_TRUE(inode->blocks == totalBlocks+1);
    EXPECT_TRUE(inode->singleIndirect = INODE_BLOCK_COUNT+1+11);
    EXPECT_TRUE(inode->doubleIndirect = INODE_BLOCK_COUNT+totalBlocks+1);
}

TEST_F(inodeTests, addDirectoryEntry){
    sType inodeDirectoryNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeDirectoryNum); 
    sType fileInode = createInode();
    inode->i_mode = S_IFDIR;
    char fileName[] = "newFile.txt";
    EXPECT_TRUE(add_directory_entry(&inode, fileInode, fileName));
    EXPECT_TRUE(directory_contains_entry(inode, fileName));
    free(inode);
}


TEST_F(inodeTests, addDirectoryEntryOnNonDirectoryINode){
    sType inodeDirectoryNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeDirectoryNum); 
    sType fileInode = createInode();
    char fileName[] = "newFile.txt";
    EXPECT_FALSE(add_directory_entry(&inode, fileInode, fileName));
    EXPECT_FALSE(directory_contains_entry(inode, fileName));
    free(inode);
}

TEST_F(inodeTests, AddingManyFilesToDirectory){
    sType inodeDirectoryNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeDirectoryNum); 
    inode->i_mode = S_IFDIR;
    char baseName[] = "newFile";
    int length;
    for(int i = 0; i < 100; i++){
        if(i < 10){
            length = 8;
        }
        else{
            length = 9;
        }
        char * fileName = ((char*)malloc(length*sizeof(char)));
        snprintf(fileName, length, "%s%d", baseName, i);
        EXPECT_TRUE(add_directory_entry(&inode, i, fileName));
        EXPECT_TRUE(directory_contains_entry(inode, fileName));
        free(fileName);
    }
    
    free(inode);
}


TEST_F(inodeTests, RemovingFromDirectory){
    sType inodeDirectoryNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeDirectoryNum); 
    inode->i_mode = S_IFDIR;

    sType maxAlloc = fs_superblock->maxAlloc;

    char baseName[] = "newFile";
    int length;
    EXPECT_TRUE(inode->blocks == 0);
    //assigning a bunch of files to the directory
    for(int i = 0; i < 10; i++){
        length = 9;
        char * fileName = ((char*)malloc(length*sizeof(char)));
        snprintf(fileName, length, "%s%d", baseName, i);
        EXPECT_TRUE(add_directory_entry(&inode, i, fileName));
        EXPECT_TRUE(directory_contains_entry(inode, fileName));
        free(fileName);
    }
    //Asserting that the free block has changed
    EXPECT_TRUE(maxAlloc == fs_superblock->maxAlloc-1);
    EXPECT_TRUE(inode->blocks == 1);

    for(int i = 0; i < 10; i++){
        length = 9;
        char * fileName = ((char*)malloc(length*sizeof(char)));
        snprintf(fileName, length, "%s%d", baseName, i);
        EXPECT_TRUE(remove_from_directory(&inode, fileName));
        EXPECT_FALSE(directory_contains_entry(inode, fileName));
        free(fileName);
    }

    EXPECT_TRUE(maxAlloc == fs_superblock->maxAlloc);
    EXPECT_TRUE(inode->blocks == 0);
}


TEST_F(inodeTests, RemovingDataBlocks){
    sType inodeNum = createInode();
    inodeStruct* inode = loadINodeFromDisk(inodeNum); 
    sType initMalloc = fs_superblock->maxAlloc;
    sType totalBlocks = NUM_DIRECT_BLOCKS+NUM_OF_SINGLE_INDIRECT_BLOCK_ADDR;
    for(int i = 1; i <= totalBlocks; i++) {
        sType block_num = allocate_data_block();
        EXPECT_TRUE(add_datablock_to_inode(inode, block_num));
    }
    printf("Initial freelist head%ld\n", fs_superblock->freelist_head);
    bool result = add_datablock_to_inode(inode, INODE_BLOCK_COUNT+totalBlocks+1);
    EXPECT_EQ(initMalloc+totalBlocks+3, fs_superblock->maxAlloc);//init malloc +total blocks added + 3 for single/double indirect block
    EXPECT_EQ(inode->blocks, totalBlocks+1);
    remove_datablocks_range_from_inode(inode, 0);
        printf("%ld\n", fs_superblock->freelist_head);

    EXPECT_EQ(inode->blocks, 0);

}

} //Namespace