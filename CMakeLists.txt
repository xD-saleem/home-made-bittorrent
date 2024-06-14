cmake_minimum_required(VERSION 3.19)
project(main)

## Dependencies
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
find_package(cryptopp REQUIRED)
find_package(loguru CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

## Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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
include/BitTorrentMessage.cpp
include/BitTorrentMessage.h
include/Block.h
include/connect.cpp
include/connect.h
include/PeerConnection.cpp
include/PeerConnection.h
include/PeerRetriever.cpp
include/PeerRetriever.h
include/Piece.cpp
include/Piece.h
include/PieceManager.cpp
include/PieceManager.h
include/SharedQueue.h
include/TorrentClient.cpp
include/TorrentClient.h
include/TorrentFileParser.cpp
include/TorrentFileParser.h
include/utils.cpp
include/utils.h
)


target_link_libraries(main PRIVATE bencoding OpenSSL::SSL OpenSSL::Crypto cpr::cpr cryptopp::cryptopp loguru fmt::fmt)

# Tests
enable_testing()
add_executable(
tests ${TEST_SRC}

# Business logic
include/utils.cpp
include/utils.h

include/TorrentFileParser.h
include/TorrentFileParser.cpp

include/BitTorrentMessage.h
include/BitTorrentMessage.cpp

# Tests
include/Main_test.cpp
include/TorrentFileParser_test.cpp
include/BitTorrentMessage_test.cpp
)

# Link libraries
target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main loguru OpenSSL::SSL)

## Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
