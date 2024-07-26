
#ifndef BITTORRENTCLIENT_TORRENTHANDLER_H
#define BITTORRENTCLIENT_TORRENTHANDLER_H

#include <string>

class TorrentHandler {
 public:
  std::string handle(const std::string& payload);
  std::string parseMagnet(const std::string& payload);
};

#endif  // BITTORRENTCLIENT_TORRENTHANDLER_H
