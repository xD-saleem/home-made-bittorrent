
#include <fmt/core.h>

#include "Torrent_Parser.h"

int main() {
  std::string torrentFilePath = "debian.torrent";
  fmt::print("Reading torrent url: {}\n", torrentFilePath);

  Torrent_Parser parser(torrentFilePath);

  fmt::print("Torrent file parsed successfully\n");

  std::string announceUrl = parser.getAnnounce();

  const std::string infoHash = parser.getInfoHash();

  std::string filename = parser.getFileName();
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;

  return 0;
};

