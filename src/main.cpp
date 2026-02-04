// Copyright 2025
#include <core/PeerRegistry.h>
#include <fmt/base.h>
#include <fmt/color.h>

#include <iostream>
#include <memory>
#include <string>
#include <tl/expected.hpp>

#include "core/PieceManager.h"
#include "core/TorrentClient.h"
#include "core/TorrentState.h"
#include "infra/DatabaseService.h"
#include "infra/DiskManager.h"
#include "infra/Logger.h"
#include "infra/Queue.h"
#include "network/PeerRetriever.h"
#include "utils/TorrentFileParser.h"

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
  auto database_service = std::make_shared<DatabaseService>(database);

  // Torrent State
  auto torrent_state = std::make_shared<TorrentState>(database_service);

  // Torrent File Parser
  auto torrent_file_parser = std::make_shared<TorrentFileParser>(download_path);

  auto downloaded_file_name = torrent_file_parser->getFileName().value();

  auto peer_registry = std::make_shared<PeerRegistry>();

  auto peer_repository =
      std::make_shared<PieceRepository>(peer_registry, torrent_file_parser);

  auto disk_manager = std::make_shared<DiskManager>();

  // Torrent Piece Manager
  auto piece_manager = std::make_shared<PieceManager>(
      torrent_file_parser, peer_registry, disk_manager, peer_repository,
      downloaded_file_name, threads);

  std::shared_ptr<Queue<std::unique_ptr<Peer>>> queue =
      std::make_shared<Queue<std::unique_ptr<Peer>>>();

  // TODO(slim): add where to save torrent
  auto torrent_client =
      TorrentClient(std::move(queue), torrent_state, piece_manager,
                    peer_registry, torrent_file_parser, threads);

  Logger::log("Parsing Torrent file " + download_path);

  torrent_client.start(download_path);

  Logger::log("Finished Downloading " + downloaded_file_name);

  return 0;
}
