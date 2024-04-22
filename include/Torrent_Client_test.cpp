#include "Torrent_Client.h"

#include <gtest/gtest.h>

TEST(TorrentClient, Print) {
  Torrent_Client tc = Torrent_Client(Torrent_Parser("debian.torrent"));
  EXPECT_EQ(tc.download(), 0);
}
