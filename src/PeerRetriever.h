#ifndef BITTORRENTCLIENT_PEERRETRIEVER_H
#define BITTORRENTCLIENT_PEERRETRIEVER_H

#include <cpr/cpr.h>

#include <tl/expected.hpp>
#include <vector>

#include "Logger.h"

struct PeerRetrieverError {
  std::string message;
};
/**
 * An representation of peers which the reponse retrieved from the tracker.
 * Contains a string that denotes the IP of the peer as well as a port number.
 */
struct Peer {
  std::string ip;
  int port;
};

/**
 * Retrieves a list of peers by sending a GET request to the tracker.
 */
class PeerRetriever {
 private:
  std::shared_ptr<Logger> logger;
  std::string announceUrl;
  std::string infoHash;
  std::string peerId;
  int port;
  const u_int64_t fileSize;

  tl::expected<std::vector<Peer *>, PeerRetrieverError> decodeResponse(
      std::string response);

 public:
  explicit PeerRetriever(std::shared_ptr<Logger> logger, std::string peerId,
                         std::string announceUrL, std::string infoHash,
                         int port, u_int64_t fileSize);
  std::vector<Peer *> retrievePeers(u_int64_t bytesDownloaded = 0);
  std::vector<Peer *> retrieveSeedPeers(u_int64_t bytesDownloaded = 0);
};

#endif  // BITTORRENTCLIENT_PEERRETRIEVER_H
