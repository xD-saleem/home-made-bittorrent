
#include <cstdint>
#include <fstream>
#include <string>

#include "core/Piece.h"

class DiskManager {
 public:
  void writePiece(Piece* piece, int64_t pieceLength);
  void allocateFile(const std::string& downloadPath, int64_t size);

  explicit DiskManager() = default;

  ~DiskManager() { downloadedFile_.close(); }

 private:
  std::ofstream downloadedFile_;
};
