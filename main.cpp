#include <fmt/core.h>

#include <functional>
#include <iostream>
#include <loguru/loguru.hpp>
#include <tl/expected.hpp>

#include "DatabaseService.h"
#include "TorrentClient.h"
#include "TorrentState.h"

int main(int argc, char* argv[]) {
  loguru::init(argc, argv);

  std::string torrentFilePath = "debian.torrent";

  LOG_F(INFO, "Starting torrent client");

  std::string filename = "torrent.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;
  std::string peerID = "peer_id";

  int workerThreadNum = 20;
  int isLoggingEnabled = true;

  std::shared_ptr<SQLite::Database> db = initDB("torrent_state.db3");

  // Database Service
  std::shared_ptr<DatabaseService> databaseSvc =
      std::make_shared<DatabaseService>(db);

  // Torrent State
  std::shared_ptr<TorrentState> torrentState =
      std::make_shared<TorrentState>(databaseSvc);

  TorrentClient torrentClient(
      // Deps.
      torrentState,
      // variables
      workerThreadNum, isLoggingEnabled, downloadDirectory);

  LOG_F(INFO, "Downloading torrent file");

  torrentClient.download(downloadPath, downloadDirectory);

  LOG_F(INFO, "Downloaded torrent file successfully");
  return 0;
};

