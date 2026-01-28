#include "infra/DiskManager.h"

#include <fmt/base.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "core/Piece.h"

TEST(DiskManager, WritePieceWritesAtCorrectPosition) {
  DiskManager dm;
  const int64_t piece_length = 4;
  const int64_t total_size = piece_length * 3;
  auto test_file = std::filesystem::temp_directory_path() / "test.torrent";

  dm.allocateFile(test_file, total_size);

  // Use a string with a known length to avoid C-string null termination issues
  std::string data = "abcd";
  std::vector<std::unique_ptr<Block>> blocks{};
  // Assuming Piece(index, blocks, data)
  Piece piece(1, std::move(blocks), data);

  dm.writePiece(&piece, piece_length);

  std::ifstream in(test_file, std::ios::binary);
  ASSERT_TRUE(in.is_open());

  std::string contents(total_size, '\0');
  in.read(contents.data(), total_size);

  std::string expected_piece_1(4, '\0');

  EXPECT_EQ(contents.substr(0, 4), expected_piece_1);

  EXPECT_EQ(std::filesystem::file_size(test_file), total_size);
}
