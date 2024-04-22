#include "Torrent_Client.h"

#include <gtest/gtest.h>

TEST(TorrentClient, Print) {
  Torrent_Client tc = Torrent_Client();
  EXPECT_EQ(tc.download(), 0);
}
