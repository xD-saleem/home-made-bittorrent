#include "BitTorrentMessage.h"

#include <fmt/core.h>
#include <gtest/gtest.h>

TEST(BittorrentMessage, init) {
  BitTorrentMessage message = BitTorrentMessage(0, "request");
  EXPECT_EQ(message.getMessageId(), 0);
}

TEST(BittorrentMessage, init2) {
  BitTorrentMessage message = BitTorrentMessage(0, "request");
  EXPECT_EQ(message.getPayload(), "request");

  std::string expected("\0\0\0\b\0request", 12);
  std::string actual = message.toString();
  EXPECT_EQ(actual, expected);
}
