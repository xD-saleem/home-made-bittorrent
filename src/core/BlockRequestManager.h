
#ifndef BITTORRENTCLIENT_BLOCKREQUESTMANAGER_H
#define BITTORRENTCLIENT_BLOCKREQUESTMANAGER_H

#include <string>
#include <vector>

#include "core/Block.h"
#include "core/PeerRegistry.h"

struct PendingRequest {
  Block* block;
  time_t timestamp;
};

class BlockRequestManager {
 private:
  // Deps
  std::vector<PendingRequest*> pendingRequests_;

 public:
  explicit BlockRequestManager();
  // Destructor
  ~BlockRequestManager() = default;

  Block* expiredRequest(const std::string& peerId, PeerRegistry& peers);

  void addPending(Block* block);

  void complete(int pieceIndex, int blockOffset);
};

#endif  // BITTORRENTCLIENT_BLOCKREQUESTMANAGER_H
