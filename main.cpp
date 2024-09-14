#include <fmt/core.h>

#include <loguru/loguru.hpp>
#include <tl/expected.hpp>

#include "DatabaseService.h"

int main(int argc, char* argv[]) {
  loguru::init(argc, argv);

  std::string torrentFilePath = "debian.torrent";

  LOG_F(INFO, "Starting torrent client");

  std::string filename = "debian.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;
  std::string peerID = "peer_id";

  int workerThreadNum = 20;
  int isLoggingEnabled = true;

  // std::unique_ptr<sqlite3, SQLiteDeleter> db = initDB("test.db");

  DatabaseService d = DatabaseService();

  // auto err = d.insertOne("id", "torrentname");

  d.getTorrent("id");

  // TorrentState torrentState;
  //
  // TorrentClient torrentClient(
  //     // Deps.
  //     &torrentState,
  //
  //     // variables
  //     workerThreadNum, isLoggingEnabled, downloadDirectory);
  //
  // LOG_F(INFO, "Downloading torrent file");
  //
  // torrentClient.downloadFile(downloadPath, downloadDirectory);
  //
  // LOG_F(INFO, "Downloaded torrent file successfully");
  return 0;
};

