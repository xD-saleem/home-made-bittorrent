
#ifndef BITTORRENTCLIENT_PIECEPICKER_H
#define BITTORRENTCLIENT_PIECEPICKER_H

#include <mutex>
#include <string>

#include "core/Piece.h"

struct PiecePickerError {
  std::string message;
};

class PieceStorage {
 private:
  // Deps
  std::mutex lock_;

 public:
  explicit PieceStorage();
  // Destructor
  ~PieceStorage() = default;

  void openFile(const std::string& downloadPath);

  void write(Piece* piece);

  uint64_t bytesDownloaded(size_t completedPieces) const;
};

#endif  // BITTORRENTCLIENT_PIECEPICKER_H
