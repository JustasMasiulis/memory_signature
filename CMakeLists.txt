cmake_minimum_required(VERSION 3.8)
project(memory_signature)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES library.cpp library.h)
add_library(memory_signature ${SOURCE_FILES})