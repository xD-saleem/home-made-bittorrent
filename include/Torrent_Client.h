#ifndef BITTORRENTCLIENT_TORRENT_CLIENT
#define BITTORRENTCLIENT_TORRENT_CLIENT

#include <string>

#include "Torrent_Parser.h"

class Torrent_Client {
 private:
  const Torrent_Parser &parser;
  const std::string &downloadPath;

 public:
  Torrent_Client(const Torrent_Parser &parser, const std::string &downloadPath);

  int download() const;
  int requestPeers(std::string &peerID, int port) const;

  // Destructor
  ~Torrent_Client() = default;
};

#endif  // BITTORRENTCLIENT_TORRENT_CLIENT

