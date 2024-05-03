
#include <fmt/core.h>

#include "Torrent_Client.h"
#include "Torrent_Parser.h"

int main() {
  std::string torrentFilePath = "debian.torrent";
  fmt::print("Reading torrent url: {}\n", torrentFilePath);

  Torrent_Parser parser(torrentFilePath);

  fmt::print("Torrent file parsed successfully\n");

  std::string filename = parser.getFileName();
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;
  std::string peerID = "peer_id";

  Torrent_Client torrentClient(parser, ref(downloadPath));

  torrentClient.requestPeers(std::ref(peerID), 6881);

  return 0;
};

