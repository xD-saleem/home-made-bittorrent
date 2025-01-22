#include "TorrentFileParser.h"

#include <gtest/gtest.h>

TEST(PieceManagerTest, getFileName) {
  TorrentFileParser tfp = TorrentFileParser("debian.torrent");
  EXPECT_EQ(tfp.getFileName(), "debian-12.5.0-amd64-netinst.iso");
}

TEST(PieceManagerTest, getFileSize) {
  TorrentFileParser tfp = TorrentFileParser("debian.torrent");
  EXPECT_EQ(tfp.getFileSize(), 659554304);
}

TEST(PieceManagerTest, getAnnounce) {
  TorrentFileParser tfp = TorrentFileParser("debian.torrent");
  EXPECT_EQ(tfp.getAnnounce(), "http://bttracker.debian.org:6969/announce");
}

TEST(PieceManagerTest, getInfoHash) {
  TorrentFileParser tfp = TorrentFileParser("debian.torrent");
  EXPECT_EQ(tfp.getInfoHash(), "2b66980093bc11806fab50cb3cb41835b95a0362");
}

TEST(PieceManagerTest, splitPieceHashes) {
  TorrentFileParser tfp = TorrentFileParser("debian.torrent");
  auto output = tfp.splitPieceHashes();

  int result = 2516;

  EXPECT_EQ(output.has_value(), true);
  EXPECT_EQ(output.value().size(), result);
}

