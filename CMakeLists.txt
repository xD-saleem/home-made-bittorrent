cmake_minimum_required(VERSION 3.19)
project(main)

find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
include_directories(lib)
include_directories(include)

# Bencoding
file(GLOB BENCODING_SRC "lib/bencode/*.cpp" "include/bencode/*.h")
add_library(bencoding STATIC ${BENCODING_SRC})

add_executable(main main.cpp include/torrent_parser.cpp include/torrent_parser.h)

target_link_libraries(main PRIVATE bencoding fmt::fmt OpenSSL::SSL OpenSSL::Crypto)

# Tests
enable_testing()
add_executable(tests ${TEST_SRC} include/torrent_parser_test.cpp include/torrent_parser.h include/torrent_parser.cpp)
target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
