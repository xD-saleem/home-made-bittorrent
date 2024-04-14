#include "torrent_parser.h"

#include <gtest/gtest.h>

// GetFileSize test
TEST(TorrentParserTest, GetFileSize) {
  torrent_parser parser("debian.torrent");
  EXPECT_EQ(parser.getFileSize(), 659554304);
}

// GetPieceLength test
TEST(TorrentParserTest, GetPieceLength) {
  torrent_parser parser("debian.torrent");
  EXPECT_EQ(parser.getPieceLength(), 3);
}

// GetFileName test
TEST(TorrentParserTest, GetFileName) {
  torrent_parser parser("debian.torrent");
  EXPECT_EQ(parser.getFileName(), "debian-12.5.0-amd64-netinst.iso");
}

// GetAnnounce test
TEST(TorrentParserTest, GetAnnounce) {
  torrent_parser parser("debian.torrent");
  EXPECT_EQ(parser.getAnnounce(), "http://bttracker.debian.org:6969/announce");
}

// GetInfoHash test
TEST(TorrentParserTest, GetInfoHash) {
  torrent_parser parser("debian.torrent");
  EXPECT_EQ(parser.getInfoHash(), "info hash");
}

// splitPieceHashes test
TEST(TorrentParserTest, splitPieceHashes) {
  torrent_parser parser("debian.torrent");
  auto res = parser.splitPieceHashes();
  EXPECT_EQ(res.value().size(), 2516);
}

int main(int argc, char **argv) {
  // Initialize Google Test framework
  ::testing::InitGoogleTest(&argc, argv);
  // Run tests
  return RUN_ALL_TESTS();
}
