#include <fmt/base.h>
#include <fmt/color.h>

#include <iostream>
#include <memory>
#include <tl/expected.hpp>

#include "DatabaseService.h"
#include "Logger.h"
#include "TorrentClient.h"
#include "TorrentFileParser.h"
#include "TorrentState.h"
#include "bencode/BInteger.h"

void customLogFunction(const std::string &message) {
  fmt::print(fg(fmt::color::cyan), "[LOG]: {}\n", message);
}

int workerThreadNum = 50;

int main(int argc, char *argv[]) {
  std::string filename = "debian.torrent";
  std::string downloadDirectory = "./";
  std::string downloadPath = downloadDirectory + filename;

  // Logger
  std::shared_ptr<Logger> logger = std::make_shared<Logger>(customLogFunction);

  std::shared_ptr<SQLite::Database> db = initDB("torrent_state.db3");

  // Database Service
  std::shared_ptr<DatabaseService> databaseSvc =
      std::make_shared<DatabaseService>(db, logger);

  // Torrent State
  std::shared_ptr<TorrentState> torrentState =
      std::make_shared<TorrentState>(databaseSvc);

  // Torrent File Parser
  std::shared_ptr<TorrentFileParser> torrentFileParser =
      std::make_shared<TorrentFileParser>(downloadPath);

  // Torrent Piece Manager
  std::shared_ptr<PieceManager> pieceManager = std::make_shared<PieceManager>(
      torrentFileParser, logger, downloadDirectory, workerThreadNum);

  TorrentClient torrentClient(logger, torrentState, pieceManager,
                              torrentFileParser,                   // deps
                              workerThreadNum, downloadDirectory); // variables

  logger->log("Parsing Torrent file " + downloadPath);

  torrentClient.start(downloadPath);

  return 0;
};
