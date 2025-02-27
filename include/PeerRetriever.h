#ifndef BITTORRENTCLIENT_PEERRETRIEVER_H
#define BITTORRENTCLIENT_PEERRETRIEVER_H

#include <cpr/cpr.h>

#include <tl/expected.hpp>
#include <vector>

#include "Logger.h"

struct PeerRetrieverError {
  std::string message;
};

struct Peer {
  std::string ip;
  int port;
};

class PeerRetriever {
private:
  std::shared_ptr<Logger> logger;
  std::string peerId;
  std::string announceUrl;
  std::string infoHash;
  int port;
  const unsigned long fileSize;

  tl::expected<std::vector<Peer *>, PeerRetrieverError>
  decodeResponse(std::string response);

public:
  explicit PeerRetriever(std::shared_ptr<Logger> logger, std::string peerId,
                         std::string announceUrl, std::string infoHash,
                         int port, unsigned long fileSize);

  tl::expected<std::vector<Peer *>, PeerRetrieverError>
  retrievePeers(unsigned long bytesDownloaded = 0);
};

#endif // BITTORRENTCLIENT_PEERRETRIEVER_H
