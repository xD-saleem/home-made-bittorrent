
#include <fmt/core.h>

#include "torrent_parser.h"

struct Dependency {};

int main() {
  std::string torrentFilePath = "debian.torrent";
  fmt::print("Reading torrent file: {}\n", torrentFilePath);

  torrent_parser parser(torrentFilePath);

  // parser.getFileName();
};

