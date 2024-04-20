#include "Torrent_Parser.h"

#include <gtest/gtest.h>

// GetFileSize test
TEST(TorrentParserTest, GetFileSize) {
  Torrent_Parser parser("debian.torrent");
  EXPECT_EQ(parser.getFileSize(), 659554304);
}

// GetPieceLength test
TEST(TorrentParserTest, GetPieceLength) {
  Torrent_Parser parser("debian.torrent");
  EXPECT_EQ(parser.getPieceLength(), 3);
}

// GetFileName test
TEST(TorrentParserTest, GetFileName) {
  Torrent_Parser parser("debian.torrent");
  EXPECT_EQ(parser.getFileName(), "debian-12.5.0-amd64-netinst.iso");
}

// GetAnnounce test
TEST(TorrentParserTest, GetAnnounce) {
  Torrent_Parser parser("debian.torrent");
  EXPECT_EQ(parser.getAnnounce(), "http://bttracker.debian.org:6969/announce");
}

// GetInfoHash test
TEST(TorrentParserTest, GetInfoHash) {
  Torrent_Parser parser("debian.torrent");
  EXPECT_EQ(parser.getInfoHash(), "info hash");
}

// splitPieceHashes test
TEST(TorrentParserTest, splitPieceHashes) {
  Torrent_Parser parser("debian.torrent");
  auto res = parser.splitPieceHashes();
  EXPECT_EQ(res.value().size(), 2516);
}

