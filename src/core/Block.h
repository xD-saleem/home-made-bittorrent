#ifndef BITTORRENTCLIENT_BLOCK_H
#define BITTORRENTCLIENT_BLOCK_H

#include <string>

enum BlockStatus { kMissing = 0, kPending = 1, kRetrieved = 2 };

struct Block {
  int piece;
  int offset;
  int length;
  BlockStatus status;
  std::string data;
};

#endif  // BITTORRENTCLIENT_BLOCK_H
