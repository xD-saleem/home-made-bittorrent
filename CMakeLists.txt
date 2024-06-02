cmake_minimum_required(VERSION 3.19)
project(main)

## Dependencies
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
find_package(cryptopp REQUIRED)
find_package(loguru CONFIG REQUIRED)

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


target_link_libraries(main PRIVATE bencoding OpenSSL::SSL OpenSSL::Crypto cpr::cpr cryptopp::cryptopp loguru)

# Tests
# enable_testing()
# add_executable(
# tests ${TEST_SRC}

# Business logic
# include/TorrentParser.h
# include/TorrentParser.cpp
#
# include/PieceManager.h
# include/PieceManager.cpp
#
# include/TorrentClient.h
# include/TorrentClient.cpp
#
# include/Piece.h
# include/Piece.cpp
#
# include/Block.h
#
# include/PeerRetriever.h
# include/PeerRetriever.cpp
#
# # Tests
# include/Main_test.cpp
# include/TorrentParser_test.cpp
# include/PieceManager_test.cpp
# include/TorrentClient_test.cpp
# )

## Link libraries
# target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

## Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
