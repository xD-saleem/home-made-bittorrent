
#include "infra/DiskManager.h"

#include <string>

#include "core/Piece.h"

void DiskManager::writePiece(Piece* piece, int64_t pieceLength) {
  int64_t position = piece->index * pieceLength;
  downloadedFile_.seekp(position);
  downloadedFile_ << piece->getData();
}

void DiskManager::allocateFile(const std::string& downloadPath, int64_t size) {
  downloadedFile_.open(downloadPath, std::ios::binary | std::ios::out);
  downloadedFile_.seekp(size - 1);
  char zero = 0;
  downloadedFile_.write(&zero, 1);
}
