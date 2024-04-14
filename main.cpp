
#include <fmt/core.h>

#include "torrent_parser.h"

int main() {
  std::string torrentFilePath = "debian.torrent";
  fmt::print("Reading torrent url: {}\n", torrentFilePath);

  torrent_parser parser(torrentFilePath);

  fmt::print("Torrent file parsed successfully\n");

  return 0;
};

