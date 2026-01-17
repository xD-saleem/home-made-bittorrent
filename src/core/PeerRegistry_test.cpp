#include "core/PeerRegistry.h"

#include <gtest/gtest.h>

TEST(PeerRegistryTest, d) {
  PeerRegistry peer_registry = PeerRegistry();

  peer_registry.addPeer("peerID", "bitRegistry");

  auto res = peer_registry.getPeer("peerID");
  EXPECT_EQ(res.value(), "bitRegistry");

  peer_registry.updatePeer("peerID", 1);

  res = peer_registry.getPeer("peerID");
  EXPECT_EQ(res.value(), "bitRegistry");

  peer_registry.removePeer("peerID");

  res = peer_registry.getPeer("peerID");
  EXPECT_EQ(res.error().message, "Attempting to get peer peerID");
}
