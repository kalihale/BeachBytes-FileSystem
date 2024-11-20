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

TEST_F(inodeTests, FreeBlock) {
    inodeStruct inode;
    inode.directAddresses[0] = INODE_BLOCK_COUNT+1; 
    inode.directAddresses[0] = 0;
    EXPECT_TRUE(free_block(&inode)); 
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


} //Namespace