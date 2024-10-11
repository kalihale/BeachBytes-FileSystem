cmake_minimum_required(VERSION 3.30)

project(cs270)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(header
    test.h
)
set(src
    test.cpp 
	 
 )

add_executable(cs270 ${src} ${header})

target_include_directories(cs270 PUBLIC ./)

