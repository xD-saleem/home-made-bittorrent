cmake_minimum_required(VERSION 3.31)
project(main)

## Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT APPLE)
    # set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()


## Dependencies
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
find_package(cryptopp REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp REQUIRED)

## Include directories
include_directories(lib)
include_directories(src)

# Bencoding
file(GLOB BENCODING_SRC "lib/bencode/*.cpp" "lib/bencode/*.h")
add_library(bencoding STATIC ${BENCODING_SRC})

set(
    CPP_TEMPLATE_SOURCES
    src/main.cpp
)
set(
    CPP_TEMPLATE_TEST_SOURCES
    src/main_test.cpp
)
# Main
add_executable(main ${CPP_TEMPLATE_SOURCES}

# Classes
src/BitTorrentMessage.cpp
src/BitTorrentMessage.h

src/Block.h

src/connect.cpp
src/connect.h

src/PeerConnection.cpp
src/PeerConnection.h

src/PeerRetriever.cpp
src/PeerRetriever.h

src/Piece.cpp
src/Piece.h

src/PieceManager.cpp
src/PieceManager.h

src/SharedQueue.h

src/TorrentClient.cpp
src/TorrentClient.h

src/TorrentFileParser.cpp
src/TorrentFileParser.h

src/TorrentState.cpp
src/TorrentState.h

src/DatabaseService.h
src/DatabaseService.cpp

src/Logger.cpp
src/Logger.h

src/utils.h
src/utils.cpp
)

target_link_libraries(main PRIVATE bencoding OpenSSL::SSL OpenSSL::Crypto cpr::cpr cryptopp::cryptopp fmt::fmt tl::expected SQLiteCpp)

# Tests
enable_testing()
add_executable(tests ${TEST_SRC}

# Business logic
src/Main_test.cpp

src/utils.cpp
src/utils.h

src/TorrentFileParser.h
src/TorrentFileParser.cpp
src/TorrentFileParser_test.cpp

src/BitTorrentMessage.h
src/BitTorrentMessage.cpp
src/BitTorrentMessage_test.cpp

src/TorrentState.h
src/TorrentState.cpp
src/TorrentState_test.cpp

src/DatabaseService.h
src/DatabaseService.cpp
src/DatabaseService_mock.h
src/DatabaseService_test.cpp

src/Logger.cpp
src/Logger.h

src/PieceManager.h
src/PieceManager.cpp
src/PieceManager_test.cpp
)

# Link libraries
target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main OpenSSL::SSL SQLiteCpp)

add_custom_target(
    check-format
    COMMAND clang-format --dry-run ${CPP_TEMPLATE_SOURCES} ${CPP_TEMPLATE_TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "checking format in src..."
)

add_custom_target(
    format
    COMMAND clang-format -i ${CPP_TEMPLATE_SOURCES} ${CPP_TEMPLATE_TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "checking format in src..."
)
