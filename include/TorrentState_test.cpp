
#include "TorrentState.h"

#include <gtest/gtest.h>

TEST(TorrentState, storeState) {
  TorrentState ts = TorrentState();
  EXPECT_EQ(ts.storeState(), "omg");
}
