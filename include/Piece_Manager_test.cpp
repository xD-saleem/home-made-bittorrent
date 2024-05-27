#include "Piece_Manager.h"

#include <fmt/core.h>
#include <gtest/gtest.h>

// TODO add tests.
TEST(PieceManagerTest, AddPeerTest) {
  Piece_Manager pm =
      Piece_Manager(Torrent_Parser("debian.torrent"), "test", 10);

  // Add a peer
  pm.addPeer("A", "B");

  // Get the peer
  auto peers = pm.getPeers();
  auto m = peers.at("A");
  EXPECT_EQ(m, "B");

  // update the peer
  pm.updatePeer("A", 2);

  // Get the peer
  peers = pm.getPeers();
  auto n = peers.at("A");
  EXPECT_EQ(n, "B");

  // Delete the peer
  pm.removePeer("A");
  EXPECT_EQ(pm.getPeers().size(), 0);
}

