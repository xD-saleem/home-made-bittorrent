#ifndef BITTORRENTCLIENT_PIECE_H
#define BITTORRENTCLIENT_PIECE_H

#include <string>
#include <tl/expected.hpp>
#include <vector>

#include "Block.h"

struct PieceError {
  std::string message;
};

class Piece {
 private:
  const std::string hashValue_;

 public:
  const int index;
  std::vector<Block*> blocks;

  explicit Piece(int index, std::vector<Block*> blocks, std::string hashValue);
  ~Piece();
  void reset();
  std::string getData();
  Block* nextRequest();
  tl::expected<void, PieceError> blockReceived(int offset, std::string data);
  bool isComplete();
  bool isHashMatching();
};

#endif  // BITTORRENTCLIENT_PIECE_H
