cmake_minimum_required(VERSION 3.29)
project(main)

## Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

## Dependencies
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
find_package(cryptopp REQUIRED)
find_package(loguru CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp REQUIRED)

## Include directories
include_directories(include)
include_directories(lib)

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

include/TorrentState.cpp
include/TorrentState.h

include/DatabaseService.h
include/DatabaseService.cpp

include/utils.h
include/utils.cpp
)

target_link_libraries(main PRIVATE bencoding OpenSSL::SSL OpenSSL::Crypto cpr::cpr cryptopp::cryptopp loguru fmt::fmt tl::expected SQLiteCpp)

# Tests
enable_testing()
add_executable(
tests ${TEST_SRC}

# Business logic
include/Main_test.cpp

include/utils.cpp
include/utils.h

include/TorrentFileParser.h
include/TorrentFileParser.cpp
include/TorrentFileParser_test.cpp

include/BitTorrentMessage.h
include/BitTorrentMessage.cpp
include/BitTorrentMessage_test.cpp

include/TorrentState.h
include/TorrentState.cpp
include/TorrentState_test.cpp

include/DatabaseService.h
include/DatabaseService.cpp
include/DatabaseService_mock.h
include/DatabaseService_test.cpp

# include/PieceManager.h
# include/PieceManager.cpp
# include/PieceManager_test.cpp
)

# Link libraries
target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main loguru OpenSSL::SSL SQLiteCpp)

## Flags for using clang
# set(CMAKE_C_COMPILER /usr/bin/clang)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++)

## Flags for using gcc
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
