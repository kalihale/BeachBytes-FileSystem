cmake_minimum_required(VERSION 3.30)

project(cs270)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(header
    fuse_interface.h
    layerZero.h
    common_config.h
    fileStructure.h
    inode.h
)
set(src
    inode.cpp
    layerZero.cpp
    fuse_interface.c
        hello_test.cc
        hello_test.cc
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

