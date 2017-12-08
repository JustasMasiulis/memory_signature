cmake_minimum_required(VERSION 3.3)
project(memory_signature)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# library includes
include_directories (${PROJECT_SOURCE_DIR}/include)
add_library (memory_signature INTERFACE)
target_include_directories (memory_signature INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_sources (memory_signature INTERFACE ${PROJECT_SOURCE_DIR}/include/memory_signature.hpp)

# compile options configuration
if(ENABLE_ASAN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O1 -fsanitize=address -fno-omit-frame-pointer")
endif()

if(ENABLE_MSAN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O1 -fsanitize=memory -fsanitize=undefined -fsanitize-memory-track-origins -fno-omit-frame-pointer")
endif()

# add catch
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    catch
    PREFIX ${CMAKE_BINARY_DIR}/catch
    GIT_REPOSITORY https://github.com/philsquared/Catch.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON)

# Includes Catch in the project:
include_directories(${CATCH_INCLUDE_DIR} ${COMMON_INCLUDES})

#configure variables
set (TEST_APP_NAME "memory_signature_test")

#set target executable
add_executable (${TEST_APP_NAME} "${PROJECT_SOURCE_DIR}/test/main.cpp")

#add the library
target_link_libraries (${TEST_APP_NAME} memory_signature)

#include custom cmake function
include ("${CMAKE_BINARY_DIR}/catch/src/catch/contrib/ParseAndAddCatchTests.cmake")

enable_testing(true)

#parse catch tests
ParseAndAddCatchTests(${TEST_APP_NAME})
