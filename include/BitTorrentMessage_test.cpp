#include "BitTorrentMessage.h"

#include <gtest/gtest.h>

TEST(BittorrentMessage, init) {
  BitTorrentMessage message = BitTorrentMessage(0, "request");
  EXPECT_EQ(message.getMessageId(), 0);
}

TEST(BittorrentMessage, init2) {
  BitTorrentMessage message = BitTorrentMessage(0, "request");
  EXPECT_EQ(message.getPayload(), "request");
}

