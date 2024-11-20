cmake_minimum_required(VERSION 3.30)

project(cs270)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)



set(header
    fuse_interface.h
    layerZero.h
    common_config.h
    inode.h
    data_block_operation.h
    linkFuseAndFS.h
)
set(src
    inode.cpp
    layerZero.cpp
    fuse_interface.c
 )

 set(src2
 setupDiskMain.cpp
 layerZero.cpp
 )
# Find fuse3 with pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 REQUIRED fuse3)

# Include directories and link options
include_directories(${FUSE3_INCLUDE_DIRS})
link_directories(${FUSE3_LIBRARY_DIRS})
add_definitions(${FUSE3_CFLAGS_OTHER})

# Add executable and link FUSE3 libraries
add_executable(cs270 ${src} ${header})


target_link_libraries(cs270 ${FUSE3_LIBRARIES})
target_include_directories(cs270 PUBLIC ./)

enable_testing()

add_executable(
        hello_test
        hello_test.cc
        inode_tests.cc
        inode.cpp
        layerZero.cpp
)
target_link_libraries(
        hello_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
set_target_properties(hello_test PROPERTIES COMPILE_OPTIONS "-g")



