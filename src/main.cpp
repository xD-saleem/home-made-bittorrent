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
#include "string"

int main(int argc, char *argv[]) {
  int threads = 50;
  std::string download_directory = "./";

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
    return 1;
  }

  std::string download_path = argv[1];
  std::cout << "You provided the file path: " << download_path << std::endl;

  // Logger
  std::shared_ptr<Logger> logger =
      std::make_shared<Logger>(Logger::custom_log_function);

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

  auto downloaded_file_name = torrent_file_parser->getFileName().value();

  // Torrent Piece Manager
  std::shared_ptr<PieceManager> piece_manager = std::make_shared<PieceManager>(
      torrent_file_parser, downloaded_file_name, threads);

  TorrentClient torrent_client =
      TorrentClient(logger, torrent_state, piece_manager, torrent_file_parser,
                    threads, download_directory);

  Logger::log("Parsing Torrent file " + download_path);

  torrent_client.start(download_path);

  Logger::log("Finished Downloading " + downloaded_file_name);

  return 0;
}
