#include <fmt/base.h>

#include <iostream>
#include <memory>
#include <tl/expected.hpp>

#include "DatabaseService.h"
#include "TorrentClient.h"
#include "TorrentFileParser.h"
#include "TorrentState.h"
#include "bencode/BInteger.h"

int main(int argc, char *argv[]) {
  std::string filename = "debian.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;

  int workerThreadNum = 200;
  int isLoggingEnabled = true;

  std::shared_ptr<SQLite::Database> db = initDB("torrent_state.db3");

  // Database Service
  std::shared_ptr<DatabaseService> databaseSvc =
      std::make_shared<DatabaseService>(db);

  // Torrent State
  std::shared_ptr<TorrentState> torrentState =
      std::make_shared<TorrentState>(databaseSvc);

  // Torrent File Parser
  std::shared_ptr<TorrentFileParser> torrentFileParser =
      std::make_shared<TorrentFileParser>(downloadPath);

  // Torrent Piece Manager
  std::shared_ptr<PieceManager> pieceManager = std::make_shared<PieceManager>(
      torrentFileParser, downloadDirectory, workerThreadNum);

  TorrentClient torrentClient(
      // Deps.
      torrentState, pieceManager, torrentFileParser,

      // variables
      workerThreadNum, isLoggingEnabled, downloadDirectory);

  std::cout << "Parsing Torrent file " + downloadPath + "..." << std::endl;

  torrentClient.start(downloadDirectory);

  return 0;
};
