cmake_minimum_required(VERSION 3.10)
project(main)

find_package(fmt REQUIRED)

add_executable(main main.cpp include/bencode_parser.cpp)
target_link_libraries(main PUBLIC fmt::fmt)

# Set include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set the build type and compiler flags
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
