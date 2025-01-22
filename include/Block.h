#ifndef BITTORRENTCLIENT_BLOCK_H
#define BITTORRENTCLIENT_BLOCK_H

#include <string>

enum BlockStatus { missing = 0, pending = 1, retrieved = 2 };

struct Block {
  int piece;
  int offset;
  int length;
  BlockStatus status;
  std::string data;
};

#endif  // BITTORRENTCLIENT_BLOCK_H
