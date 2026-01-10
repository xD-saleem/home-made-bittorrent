#ifndef BITTORRENTCLIENT_PEERRETRIEVER_H
#define BITTORRENTCLIENT_PEERRETRIEVER_H

#include <cpr/cpr.h>

#include <tl/expected.hpp>
#include <vector>

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
  std::string announceUrl_;
  std::string infoHash_;
  std::string peerId_;
  int port_;
  const u_int64_t fileSize_;

  tl::expected<std::vector<std::unique_ptr<Peer>>, PeerRetrieverError>
  decodeResponse(std::string response);

 public:
  explicit PeerRetriever(std::string peerId, std::string announceUrL,
                         std::string infoHash, int port, u_int64_t fileSize);
  std::vector<std::unique_ptr<Peer>> retrievePeers(
      u_int64_t bytesDownloaded = 0);
};

#endif  // BITTORRENTCLIENT_PEERRETRIEVER_H
