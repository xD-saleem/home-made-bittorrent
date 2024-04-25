#ifndef BITTORRENTCLIENT_PIECE_MANAGER
#define BITTORRENTCLIENT_PIECE_MANAGER

/* PieceManager is responsible for managing and keeping track of all the pieces
of the connected peers as well as other peers which might have been
disconnected. */

class Piece_Manager {
 private:
  // Private member variables
  int piece_count;

 public:
  explicit Piece_Manager();
  int print() const;
  int get_piece_count() const;
  void set_piece_count(int piece_count);

  // Constructor to destroy the object once it goes out of scope
  ~Piece_Manager() = default;
};

#endif  // BITTORRENTCLIENT_PIECE_MANAGER
