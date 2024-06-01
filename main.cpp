
#include <fmt/core.h>

#include "TorrentClient.h"

int main() {
  std::string torrentFilePath = "debian.torrent";
  fmt::print("Reading torrent url: {}\n", torrentFilePath);

  fmt::print("Torrent file parsed successfully\n");

  std::string filename = "debian.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;
  std::string peerID = "peer_id";

  TorrentClient torrentClient(20, true, "./");
  fmt::print("Downloading file: {}\n", downloadPath);
  torrentClient.downloadFile(downloadPath, downloadDirectory);

  return 0;
};

