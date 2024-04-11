cmake_minimum_required(VERSION 3.10)
project(main)

# Find required libraries
# find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(LibtorrentRasterbar CONFIG REQUIRED)

include_directories(include)

# Bencoding
file(GLOB BENCODING_SRC "lib/bencode/*.cpp" "include/bencode/*.h")
add_library(bencoding STATIC ${BENCODING_SRC})

# Set include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add the main executable
add_executable(main main.cpp)

# Link fmt and Google Test libraries to the main executable
# target_link_libraries(main PUBLIC OpenSSL::SSL OpenSSL::Crypto fmt::fmt GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main, LibtorrentRasterbar::torrent-rasterbar)


target_link_libraries(main PUBLIC OpenSSL::SSL OpenSSL::Crypto fmt::fmt LibtorrentRasterbar::torrent-rasterbar)
# Enable testing
enable_testing()

# Add test executable for bencode_parser_test.cpp
# add_executable(bencode_parser_test)

# Link fmt and Google Test libraries to the test executable
# target_link_libraries(bencode_parser_test PRIVATE fmt::fmt GTest::gtest GTest::gtest_main)

# Register the test
# add_test(NAME bencode_parser_test COMMAND bencode_parser_test)

# Set the build type and compiler flags
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

project(main)
