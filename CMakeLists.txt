cmake_minimum_required(VERSION 2.8.3)
project(bst)

#ds determine build type, default build type RELEASE
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif(NOT CMAKE_BUILD_TYPE)
message("build type: ${CMAKE_BUILD_TYPE}")



#ds output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
message("output directory example binaries: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

#ds add c++11 support, no optimization, all warnings
add_definitions(-O3 -march=native -Wall -pedantic -std=c++11)

#ds set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/types)



#ds example binaries
add_executable(plain_search src/examples/plain_search.cpp)
