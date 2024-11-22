#include <gtest/gtest.h>
#include "../header/common_config.h"
#include "../header/layerZero.h"
#include "../header/inode.h"
#include "../header/linkFuseAndFS.h"
#include "../header/fuse_interface.h"

namespace {
class fuseTest: public::testing::Test{
protected:

    void SetUp() override {
        fs_init();
    }

    void TearDown() override {
        fs_close();
    }

};

TEST_F(fuseTest, mkdirRmdir_fsSystem){
    EXPECT_FALSE(fs_mkdir("./trynew", 666));
    EXPECT_FALSE(system("rmdir ./trynew"));
}

TEST_F(fuseTest, mkdirRmdir_systemFS){
    EXPECT_FALSE(system("mkdir ./newnew"));
    EXPECT_FALSE(fs_unlink("./newnew"));
}

TEST_F(fuseTest, mkdirRmdir_allSystem){
    EXPECT_FALSE(system("mkdir ./new"));
    EXPECT_FALSE(system("rmdir ./new"));
}

TEST_F(fuseTest, mkdirRmdir_allfs){
    EXPECT_FALSE(fs_mkdir("./try", 666));
    EXPECT_FALSE(fs_unlink("./try"));
}

// TODO I don't think create_new_file in linkFuseAndFS is being called when I run this test
TEST_F(fuseTest, catfile){
    EXPECT_FALSE(system("echo 'hello world' > new.txt"));
}


}