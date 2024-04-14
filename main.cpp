
#include <fmt/core.h>

#include "torrent_parser.h"

int main() {
  std::string torrentFilePath = "debian.torrent";
  fmt::print("Reading torrent file: {}\n", torrentFilePath);

  torrent_parser parser(torrentFilePath);

  int a = parser.getFileSize();
  fmt::print("File size: {}\n", a);
};

