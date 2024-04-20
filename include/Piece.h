#ifndef BITTORRENTCLIENT_PIECE
#define BITTORRENTCLIENT_PIECE

class Piece {
 private:
 public:
  explicit Piece();

  // Constructor to destroy the object once it goes out of scope
  ~Piece() = default;
};

#endif  // BITTORRENTCLIENT
