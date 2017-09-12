cmake_minimum_required(VERSION 3.3)
project(memory_signature)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories (${PROJECT_SOURCE_DIR}/include)
add_library (memory_signature INTERFACE)
target_include_directories (memory_signature INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_sources (memory_signature INTERFACE ${PROJECT_SOURCE_DIR}/include/memory_signature.hpp)

enable_testing()
add_subdirectory (test)
