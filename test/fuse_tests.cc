#include <gtest/gtest.h>

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

TEST_F(fuseTest, open){
    
}