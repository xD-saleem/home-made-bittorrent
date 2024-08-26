
#ifndef BITTORRENTCLIENT_TORRENTSTATE_H
#define BITTORRENTCLIENT_TORRENTSTATE_H

#include <string>

struct TorrentStateError {
  std::string message;
};

class TorrentState {
 private:
 public:
  explicit TorrentState();
  std::string storeState();

  // deconsutrctor
  ~TorrentState();
};

#endif  // BITTORRENTCLIENT_TORRENTSTATE_H
