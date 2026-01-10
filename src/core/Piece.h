#ifndef BITTORRENTCLIENT_PIECE_H
#define BITTORRENTCLIENT_PIECE_H

#include <memory>
#include <string>
#include <tl/expected.hpp>
#include <vector>

#include "core/Block.h"

struct PieceError {
  std::string message;
};

class Piece {
 private:
  const std::string hash_value_;

 public:
  const int index;
  std::vector<std::unique_ptr<Block>> blocks;

  explicit Piece(int index, std::vector<std::unique_ptr<Block>> blocks,
                 std::string hashValue);

  ~Piece() = default;
  void reset();
  std::string getData();
  Block* nextRequest();
  tl::expected<void, PieceError> blockReceived(int offset, std::string data);
  bool isComplete() const;
  bool isHashMatching();
};

#endif  // BITTORRENTCLIENT_PIECE_H
