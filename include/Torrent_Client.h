#ifndef BITTORRENTCLIENT_TORRENT_CLIENT
#define BITTORRENTCLIENT_TORRENT_CLIENT

class Torrent_Client {
 private:
 public:
  explicit Torrent_Client();
  int download() const;

  // Constructor to destroy the object once it goes out of scope
  ~Torrent_Client() = default;
};

#endif  // BITTORRENTCLIENT_TORRENT_CLIENT
