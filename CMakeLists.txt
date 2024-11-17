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
    data_block_operation.h
    linkFuseAndFS.h
)
set(src
    inode.cpp
    layerZero.cpp
    fuse_interface.cpp
    data_block_operation.cpp
    linkFuseAndFS.cpp
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
add_executable(setUpDisk ${src2} ${header})


target_link_libraries(cs270 ${FUSE3_LIBRARIES})
target_include_directories(cs270 PUBLIC ./)

