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
  std::string announceUrl;
  std::string infoHash;
  std::string peerId;
  int port;
  const unsigned long fileSize;

  tl::expected<std::vector<Peer *>, PeerRetrieverError>
  decodeResponse(std::string response);

public:
  explicit PeerRetriever(std::string peerId, std::string announceUrL,
                         std::string infoHash, int port,
                         unsigned long fileSize);
  std::vector<Peer *> retrievePeers(unsigned long bytesDownloaded = 0);
  std::vector<Peer *> retrieveSeedPeers(unsigned long bytesDownloaded = 0);
};

#endif // BITTORRENTCLIENT_PEERRETRIEVER_H
