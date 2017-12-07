cmake_minimum_required(VERSION 3.3)
project(memory_signature)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories (${PROJECT_SOURCE_DIR}/include)
add_library (memory_signature INTERFACE)
target_include_directories (memory_signature INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_sources (memory_signature INTERFACE ${PROJECT_SOURCE_DIR}/include/memory_signature.hpp)

if(ENABLE_SANITIZERS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O1 -fsanitize=address -fsanitize=memory -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer")
endif()

enable_testing()
add_subdirectory (test)
