cmake_minimum_required(VERSION 3.29)
project(main)

## Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

    # Core Logic
    src/core/Block.h
    src/core/Piece.h
    src/core/Piece.cpp
    src/core/PieceManager.h
    src/core/PieceManager.cpp
    src/core/TorrentClient.h
    src/core/TorrentClient.cpp
    src/core/TorrentState.h
    src/core/TorrentState.cpp

    # Infrastructure & Storage
    src/infra/DatabaseService.h
    src/infra/DatabaseService.cpp
    src/infra/Logger.h
    src/infra/Logger.cpp
    src/infra/Queue.h
    src/utils/TorrentFileParser.h
    src/utils/TorrentFileParser.cpp

    # Network
    src/network/BitTorrentMessage.h
    src/network/BitTorrentMessage.cpp
    src/network/connect.h
    src/network/connect.cpp
    src/network/PeerConnection.h
    src/network/PeerConnection.cpp
    src/network/PeerRetriever.h
    src/network/PeerRetriever.cpp

    # Utils
    src/utils/utils.h
    src/utils/utils.cpp
)

target_link_libraries(main PRIVATE bencoding OpenSSL::SSL OpenSSL::Crypto cpr::cpr cryptopp::cryptopp fmt::fmt tl::expected SQLiteCpp)

# Tests
enable_testing()
add_executable(tests
    src/main_test.cpp

    # Utils & Infra used in tests
    src/utils/utils.h
    src/utils/utils.cpp

    src/infra/Logger.h
    src/infra/Logger.cpp
    src/infra/DatabaseService.h
    src/infra/DatabaseService.cpp
    src/infra/DatabaseService_mock.h
    src/infra/DatabaseService_test.cpp

    # Parsers and Logic
    src/utils/TorrentFileParser.h
    src/utils/TorrentFileParser.cpp
    src/utils/TorrentFileParser_test.cpp

    # Network Logic
    src/network/BitTorrentMessage.h
    src/network/BitTorrentMessage.cpp
    src/network/BitTorrentMessage_test.cpp

    # Core State
    src/core/TorrentState.h
    src/core/TorrentState.cpp
    src/core/TorrentState_test.cpp

    # # Piece Management (uncommented as per your structure)
    # src/core/PieceManager.h
    # src/core/PieceManager.cpp
    # src/core/PieceManager_test.cpp
)

target_link_libraries(tests PRIVATE bencoding fmt::fmt GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main OpenSSL::SSL SQLiteCpp)

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
