
#include <cstdint>
#include <string>

class DiskManager {
 public:
  static void writePiece(int index, const std::string& data);
  static void readPiece(int index);
  static void allocateFile(int64_t size);
  explicit DiskManager();  // Constructor with DI

  ~DiskManager() = default;

 private:
};
