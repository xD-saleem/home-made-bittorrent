#include "Torrent_Client.h"

#include <gtest/gtest.h>

#include <functional>

TEST(TorrentClient, Print) {
  Torrent_Client tc =
      Torrent_Client(Torrent_Parser("debian.torrent"), "downloadPath");
  EXPECT_EQ(tc.download(), 0);
}
TEST(TorrentClient, RequestPeers) {
  Torrent_Client tc =
      Torrent_Client(Torrent_Parser("debian.torrent"), "downloadPath");
  std::string peerID = "peer_id";
  EXPECT_EQ(tc.requestPeers(std::ref(peerID), 6881), 0);
}
