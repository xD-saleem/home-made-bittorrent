#include "Piece_Manager.h"

#include <gtest/gtest.h>

TEST(PieceManagerTest, Print) {
  Piece_Manager pm = Piece_Manager();
  EXPECT_EQ(pm.print(), 0);
}
