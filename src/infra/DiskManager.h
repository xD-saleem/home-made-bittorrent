#pragma once
#include <cstdint>
#include <fstream>
#include <string>

#include "core/Piece.h"

class DiskManager {
 public:
  explicit DiskManager() = default;

  void writePiece(Piece* piece, int64_t pieceLength);
  void allocateFile(const std::string& downloadPath, int64_t size);

  ~DiskManager() { downloadedFile_.close(); }

 private:
  std::ofstream downloadedFile_;
};
