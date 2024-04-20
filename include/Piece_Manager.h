#ifndef BITTORRENTCLIENT_PIECE_MANAGER
#define BITTORRENTCLIENT_PIECE_MANAGER

class Piece_Manager {
 private:
 public:
  explicit Piece_Manager();
  int print() const;

  // Constructor to destroy the object once it goes out of scope
  ~Piece_Manager() = default;
};

#endif  // BITTORRENTCLIENT_PIECE_MANAGER
