
#ifndef BITTORRENTCLIENT_PIECETRACKER_H
#define BITTORRENTCLIENT_PIECETRACKER_H

#include <mutex>
#include <string>

#include "core/Piece.h"

struct PieceTrackerError {
  std::string message;
};

class PeerTracker {
 private:
  // Deps
  std::mutex lock_;

 public:
  explicit PeerTracker();
  // Destructor
  ~PeerTracker() = default;

  std::vector<std::unique_ptr<Piece>> initiatePieces();

  bool isComplete();

  std::vector<Piece*> getPieces();

  Piece* findOngoingPiece(int pieceIndex);

  void moveToOngoing(Piece* piece);
  void moveToCompleted(Piece* piece);

  tl::expected<Piece*, PieceTrackerError()> applyBlock(int pieceIndex,
                                                       int blockOffset,
                                                       std::string data);
};

#endif  // BITTORRENTCLIENT_PIECETRACKER_H
