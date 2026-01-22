
#include "DiskManager.h"

#include <string>

DiskManager::DiskManager() = default;

void DiskManager::writePiece(int index, const std::string& data) {}
void DiskManager::readPiece(int index) {}

void DiskManager::allocateFile(int64_t size) {
  downloadedFile_.open(downloadPath, std::ios::binary | std::ios::out);
  int64_t file_size = fileParser->getFileSize().value();
  downloadedFile_.seekp(file_size - 1);
  downloadedFile_.write("", 1);
}
