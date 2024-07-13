#include <iostream>
#include <loguru/loguru.hpp>
#include <tl/expected.hpp>

#include "TorrentClient.h"

int main(int argc, char* argv[]) {
  if (__cplusplus == 202101L)
    std::cout << "C++23";
  else if (__cplusplus == 202002L)
    std::cout << "C++20";
  else if (__cplusplus == 201703L)
    std::cout << "C++17";
  else if (__cplusplus == 201402L)
    std::cout << "C++14";
  else if (__cplusplus == 201103L)
    std::cout << "C++11";
  else if (__cplusplus == 199711L)
    std::cout << "C++98";
  else
    std::cout << "pre-standard C++." << __cplusplus;
  std::cout << "\n";

  // TODO magnetic

  std::string torrentFilePath = "generateTorrent.torrent";
  loguru::init(argc, argv);

  LOG_F(INFO, "Starting torrent client");

  std::string filename = "generateTorrent.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;
  std::string peerID = "peer_id";

  TorrentClient torrentClient(20, true, "./");

  LOG_F(INFO, "Downloading torrent file");

  torrentClient.downloadFile(downloadPath, downloadDirectory);

  LOG_F(INFO, "Downloaded torrent file successfully");
  return 0;
};

