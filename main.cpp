#include <tl/expected.hpp>

#include "DatabaseService.h"
#include "TorrentClient.h"
#include "TorrentState.h"

int main(int argc, char *argv[]) {
  std::string torrentFilePath = "debian.torrent";

  std::string filename = "debian.torrent";
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

  torrentClient.start(downloadPath, downloadDirectory);

  return 0;
};
