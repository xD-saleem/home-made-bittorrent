#include "TorrentHandler.h"

#include <gtest/gtest.h>

TEST(TorrentHandler, handle) {
  TorrentHandler th = TorrentHandler();
  EXPECT_EQ(th.handle("fakeString"), "TODO");
};

TEST(TorrentHandler, parseMagneticLink) {
  TorrentHandler th = TorrentHandler();
  EXPECT_EQ(th.parseMagnet("fakeString"), "TODO");
};

