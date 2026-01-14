
#ifndef BITTORRENTCLIENT_PIECEPICKER_H
#define BITTORRENTCLIENT_PIECEPICKER_H

#include <mutex>
#include <string>

#include "core/Piece.h"

struct PiecePickerError {
  std::string message;
};

class PiecePicker {
 private:
  // Deps
  std::mutex lock_;

 public:
  explicit PiecePicker();
  // Destructor
  ~PiecePicker() = default;

  Block* nextRequest(const std::string& peerId);

  Block* nextOngoing(const std::string& peerId);

  Piece* getRarestPiece(const std::string& peerId);
};

#endif  // BITTORRENTCLIENT_PIECEPICKER_H
