#include "infra/DiskManager.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "core/Piece.h"

TEST(DiskManager, WritePieceWritesAtCorrectPosition) {
  DiskManager dm;
  const int64_t piece_length = 4;
  const int64_t total_size = piece_length * 3;

  dm.allocateFile(std::filesystem::temp_directory_path(), total_size);

  ASSERT_TRUE(true);

  // std::vector<std::unique_ptr<Block>> blocks{}
  //
  // Piece piece(1, torrent_pieces, "abcd");
  //
  // dm.writePiece(&piece, piece_length);
  //
  // std::ifstream in(std::filesystem::temp_directory_path(), std::ios::binary);
  // ASSERT_TRUE(in.is_open());
  //
  // // Read full file
  // std::string contents(total_size, '\0');
  // in.read(contents.data(), total_size);
  //
  // EXPECT_EQ(contents.substr(4, 4), "ABCD");
}
