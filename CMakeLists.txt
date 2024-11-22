cmake_minimum_required(VERSION 3.30)

project(cs270)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)



set(header
    header/fuse_interface.h
    header/layerZero.h
    header/common_config.h
    header/inode.h
    header/data_block_operation.h
    header/linkFuseAndFS.h
)
set(src
        src/layerZero.cpp

    src/inode.cpp
    src/fuse_interface.cpp
    src/data_block_operation.cpp
    src/linkFuseAndFS.cpp
 )

 set(src2
        src/setupDiskMain.cpp
        src/layerZero.cpp
 )
# Find fuse3 with pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 REQUIRED fuse3)

find_package(GTest CONFIG REQUIRED)

# Include directories and link options
include_directories(${FUSE3_INCLUDE_DIRS})
link_directories(${FUSE3_LIBRARY_DIRS})
add_definitions(${FUSE3_CFLAGS_OTHER})

# Add executable and link FUSE3 libraries
add_executable(cs270 ${src} ${header})
add_executable(setUpDisk ${src2} ${header})

include_directories(/usr/include)

include(FetchContent)
FetchContent_Declare(
  googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


target_link_libraries(cs270 ${FUSE3_LIBRARIES})
target_include_directories(cs270 PUBLIC ./)

enable_testing()

add_executable(
        hello_test
        test/hello_test.cc
        test/inode_tests.cc
        src/inode.cpp
        src/layerZero.cpp
        src/data_block_operation.cpp
        header/common_config.h
        src/linkFuseAndFS.cpp
)
target_link_libraries(
        hello_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
set_target_properties(hello_test PROPERTIES COMPILE_OPTIONS "-g")



