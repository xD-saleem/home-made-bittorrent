#include "torrent_parser.h"

#include <gtest/gtest.h>

// Test case for getFileSize method
TEST(TorrentParserTest, GetFileSize) {
  // Create a torrent_parser object
  torrent_parser parser("debian.torrent");

  // Assuming getFileSize returns 3
  EXPECT_EQ(parser.getFileSize(), 3);
}

// Example of more test cases can be added
// TEST(TorrentParserTest, AnotherTest) { ... }

int main(int argc, char **argv) {
  // Initialize Google Test framework
  ::testing::InitGoogleTest(&argc, argv);
  // Run tests
  return RUN_ALL_TESTS();
}
