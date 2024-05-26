cmake_minimum_required(VERSION 3.19)
project(main)

## Dependencies
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
find_package(cryptopp REQUIRED)

## Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

## Include directories
include_directories(lib)
include_directories(include)

# Bencoding
file(GLOB BENCODING_SRC "lib/bencode/*.cpp" "include/bencode/*.h")
add_library(bencoding STATIC ${BENCODING_SRC})

# Main
add_executable(main main.cpp

## Classes
include/Torrent_Parser.cpp
include/Torrent_Parser.h

include/Piece_Manager.cpp
include/Piece_Manager.h

include/Torrent_Client.cpp
include/Torrent_Client.h

include/Piece.h
include/Piece.cpp

include/Block.h
)

target_link_libraries(main PRIVATE bencoding fmt::fmt OpenSSL::SSL OpenSSL::Crypto cpr::cpr cryptopp::cryptopp)

# Tests
enable_testing()
add_executable(
tests ${TEST_SRC}

# Business logic
include/Torrent_Parser.h
include/Torrent_Parser.cpp

include/Piece_Manager.h
include/Piece_Manager.cpp

include/Torrent_Client.h
include/Torrent_Client.cpp

include/Piece.h
include/Piece.cpp

include/Block.h

# Tests
include/Main_test.cpp
include/Torrent_Parser_test.cpp
include/Piece_Manager_test.cpp
include/Torrent_Client_test.cpp

)

## Link libraries
target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

## Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
