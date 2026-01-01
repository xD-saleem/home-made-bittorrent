// Copyright 2025
#include <fmt/base.h>
#include <fmt/color.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include <utility>

#include "DatabaseService.h"
#include "Logger.h"
#include "Queue.h"
#include "TorrentClient.h"
#include "TorrentFileParser.h"
#include "TorrentState.h"

int main(int argc, char* argv[]) {
  int threads = 50;
  std::string download_directory = "./";

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
    return 1;
  }

  std::string download_path = argv[1];
  Logger::log("You provided the file path: " + download_path);

  std::shared_ptr<SQLite::Database> database = initDB("torrent_state.db3");

  // Database Service
  std::shared_ptr<DatabaseService> database_service =
      std::make_shared<DatabaseService>(database);

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

  std::shared_ptr<Queue<std::unique_ptr<Peer>>> queue =
      std::make_shared<Queue<std::unique_ptr<Peer>>>();

  TorrentClient torrent_client =
      // TODO(slim): add where to save torrent
      TorrentClient(std::move(queue), torrent_state, piece_manager,
                    torrent_file_parser, threads);

  Logger::log("Parsing Torrent file " + download_path);

  torrent_client.start(download_path);

  Logger::log("Finished Downloading " + downloaded_file_name);

  return 0;
}
