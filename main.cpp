

#include <loguru/loguru.hpp>

#include "TorrentClient.h"

int main(int argc, char* argv[]) {
  std::string torrentFilePath = "debian.torrent";
  loguru::init(argc, argv);

  LOG_F(INFO, "Starting torrent client");

  std::string filename = "debian.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;
  std::string peerID = "peer_id";

  TorrentClient torrentClient(20, true, "./");

  LOG_F(INFO, "Downloading torrent file");

  torrentClient.downloadFile(downloadPath, downloadDirectory);

  LOG_F(INFO, "Downloaded torrent file successfully");
  return 0;
};

