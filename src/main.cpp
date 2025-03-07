// Copyright 2025
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
#include "string"

using std::shared_ptr;

void custom_log_function(const std::string &message) {
  fmt::print(fg(fmt::color::cyan), "[LOG]: {}\n", message);
}

int main() {
  int threads = 50;
  std::string filename = "debian.torrent";
  std::string download_directory = "./";
  std::string download_path = download_directory + filename;

  // Logger
  shared_ptr<Logger> logger = std::make_shared<Logger>(custom_log_function);

  std::shared_ptr<SQLite::Database> database = initDB("torrent_state.db3");

  // Database Service
  std::shared_ptr<DatabaseService> database_service =
      std::make_shared<DatabaseService>(database, logger);

  // Torrent State
  std::shared_ptr<TorrentState> torrent_state =
      std::make_shared<TorrentState>(database_service);

  // Torrent File Parser
  std::shared_ptr<TorrentFileParser> torrent_file_parser =
      std::make_shared<TorrentFileParser>(download_path);

  // Torrent Piece Manager
  std::shared_ptr<PieceManager> piece_manager = std::make_shared<PieceManager>(
      torrent_file_parser, logger, download_directory, threads);

  TorrentClient torrent_client =
      TorrentClient(logger, torrent_state, piece_manager, torrent_file_parser,
                    threads, download_directory);

  logger->log("Parsing Torrent file " + download_path);

  torrent_client.start(download_path);

  return 0;
}
