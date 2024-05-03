#ifndef BITTORRENTCLIENT_TORRENT_CLIENT
#define BITTORRENTCLIENT_TORRENT_CLIENT

#include "Torrent_Parser.h"

class Torrent_Client {
 private:
  const Torrent_Parser& parser;
  const std::string& downloadPath;

 public:
  explicit Torrent_Client(const Torrent_Parser& parser,
                          const std::string& downloadPath)
      : parser(parser), downloadPath(downloadPath) {}

  int download() const;
  int requestPeers(std::string& peerID, int port) const;

  // Constructor to destroy the object once it goes out of scope
  ~Torrent_Client() = default;
};

#endif  // BITTORRENTCLIENT_TORRENT_CLIENT
