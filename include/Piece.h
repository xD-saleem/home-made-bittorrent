#ifndef BITTORRENTCLIENT_PIECE_H
#define BITTORRENTCLIENT_PIECE_H

#include <vector>

#include "Block.h"

class Piece {
 private:
 public:
  const int index;
  std::vector<Block*> blocks;
  std::string hashValue;

  explicit Piece(int index, std::vector<Block*> blocks, std::string hashValue);
  ~Piece();
  void reset();
  std::string getData();
  Block* nextRequest();
  void blockReceived(int offset, std::string data);
  bool isComplete();
  bool isHashMatching();
};

#endif  // BITTORRENTCLIENT_PIECE_H
