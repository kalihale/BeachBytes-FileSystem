#include <gtest/gtest.h>
#include "common_config.h"
#include "layerZero.h"
#include "inode.h"

namespace {
class inodeTests: public::testing::Test{
protected:

    void SetUp() override {
        fs_init();
    }

};

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

/*
TEST_F(inodeTests, LoadINodeFromDisk) {
    EXPECT_EQ(load_iNode_From_Disk(-1), nullptr);
    // Load a valid inode (assuming it exists and fs_read_block works correctly)
    sType inodeNum = 5;
    inodeStruct* inode = load_iNode_From_Disk(inodeNum);
    ASSERT_NE(inode, nullptr);  // Verify inode is loaded
    // Optionally check values in the inode
    free(inode); // Clean up allocated memory
} */

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
    EXPECT_GE(inodeNum, 0); 
}

TEST_F(inodeTests, FreeBlock) {
    inodeStruct inode;
    inode.directAddresses[0] = INODE_BLOCK_COUNT+1; 
    inode.directAddresses[0] = 0;
    EXPECT_TRUE(free_block(&inode)); 
}
TEST_F(inodeTests, DeleteInode) {
    sType inodeNum = createInode();
    EXPECT_TRUE(delete_inode(inodeNum)); 
}

} //Namespace