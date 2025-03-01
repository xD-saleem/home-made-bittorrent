#include <fmt/base.h>
#include <fmt/color.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <tl/expected.hpp>

#include "DatabaseService.h"
#include "Logger.h"
#include "PeerRetriever.h"
#include "TorrentClient.h"
#include "TorrentFileParser.h"
#include "TorrentState.h"
#include "bencode/BInteger.h"
#include "exception"

void customLogFunction(const std::string &message) {
  fmt::print(fg(fmt::color::cyan), "[LOG]: {}\n", message);
}

int workerThreadNum = 50;

int torrentPort = 9000;

void panic(const std::string message) {
  std::cerr << "Panic: " << message << std::endl;
  std::terminate();
}

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

  // GeneratePeerID
  std::random_device rd;
  std::uniform_int_distribution<> distrib(1, 9);
  std::mt19937 gen(rd());

  std::string peerId = "";
  for (int i = 0; i < 12; ++i) {
    peerId += std::to_string(distrib(gen));
  }

  tl::expected<std::string, TorrentFileParserError> announceUrlResult =
      torrentFileParser->getAnnounce();

  if (!announceUrlResult) {
    panic("failed to get url result");
  }

  std::string infoHash = torrentFileParser->getInfoHash();
  tl::expected<long, TorrentFileParserError> fileSizeResult =
      torrentFileParser->getFileSize();

  if (!fileSizeResult) {
    panic("failed to get file size");
  }

  fmt::println("PEERID {}", peerId);
  PeerRetriever peerRetriever(logger, peerId, announceUrlResult.value(),
                              infoHash, torrentPort, fileSizeResult.value());

  std::shared_ptr<PeerRetriever> pr =
      std::make_shared<PeerRetriever>(peerRetriever);

  TorrentClient torrentClient(logger, torrentState, pieceManager,
                              torrentFileParser, pr, peerId, workerThreadNum,
                              downloadDirectory); // variables

  logger->log("Parsing Torrent file " + downloadPath);

  torrentClient.start(downloadDirectory);

  return 0;
};
