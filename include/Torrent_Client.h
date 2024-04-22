#ifndef BITTORRENTCLIENT_TORRENT_CLIENT
#define BITTORRENTCLIENT_TORRENT_CLIENT

#include "Torrent_Parser.h"

class Torrent_Client {
 private:
  const Torrent_Parser& parser;

 public:
  explicit Torrent_Client(const Torrent_Parser& parser) : parser(parser) {}
  int download() const;

  // Constructor to destroy the object once it goes out of scope
  ~Torrent_Client() = default;
};

#endif  // BITTORRENTCLIENT_TORRENT_CLIENT
