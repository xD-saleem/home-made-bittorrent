#include "TorrentClient.h"

#include <bencode/bencoding.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <utility>

#include "PeerConnection.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "TorrentFileParser.h"

#define PORT 8080
#define PEER_QUERY_INTERVAL 60  // 1 minute

TorrentClient::TorrentClient(
    std::shared_ptr<TorrentState> torrentState,
    std::shared_ptr<PieceManager> pieceManager,
    std::shared_ptr<TorrentFileParser> torrentFileParser, int threadNum)
    : torrentState_(std::move(torrentState)),
      pieceManager_(std::move(pieceManager)),
      torrentFileParser_(std::move(torrentFileParser)),
      threadNum_(threadNum),
      peerId_("-UT2021-") {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 9);

  if (!this->torrentState_) {
    Logger::log("torrentState is null!");
    throw std::runtime_error("torrentState is null");
  }
  if (!this->torrentFileParser_) {
    Logger::log("torrentFileParser is null!");
    throw std::runtime_error("torrentFileParser is null");
  }
  if (!this->pieceManager_) {
    Logger::log("pieceManager is null!");
    throw std::runtime_error("pieceManager is null");
  }

  for (int i = 0; i < 12; ++i) {
    peerId_ += std::to_string(distrib(gen));
  }
}

/**
 * Ensures that all resources are freed when TorrentClient is destroyed
 */
TorrentClient::~TorrentClient() = default;

void TorrentClient::start(const std::string& downloadDirectory) {
  const std::string info_hash = torrentFileParser_->getInfoHash();

  const tl::expected<std::string, TorrentFileParserError> filename_exp =
      torrentFileParser_->getFileName();

  if (!filename_exp) {
    Logger::log("Failed to get torrent filename.");
    return;
  }
  const std::string& filename = filename_exp.value();

  const auto state = torrentState_->getState(info_hash);
  if (!state) {
    Logger::log("No state found for this info hash.");
    return;
  }

  // TODO(slim): enable this once we have all the bugs/refactors done.
  //  if (state->id == info_hash) {
  //    Logger::log("Torrent already downloaded.");
  //    return;
  //  }

  downloadFile(downloadDirectory);

  auto res = torrentState_->storeState(info_hash, filename);

  if (!res) {
    Logger::log("Failed to store torrent state.");
    return;
  }
}

void TorrentClient::downloadFile(const std::string& torrentFile) {
  const auto announce_url = torrentFileParser_->getAnnounce().value();
  const auto file_size = torrentFileParser_->getFileSize().value();
  const auto info_hash = torrentFileParser_->getInfoHash();
  const auto file_name = torrentFileParser_->getFileName().value();

  // Initialize peer connections and threads
  connections_.reserve(threadNum_);
  threadPool_.reserve(threadNum_);

  for (int i = 0; i < threadNum_; ++i) {
    PeerConnection connection(&queue_, peerId_, info_hash, pieceManager_);
    // TODO(slim): fix storing raw pointer.
    connections_.push_back(&connection);

    threadPool_.emplace_back(&PeerConnection::start, connection);
  }

  auto last_peer_query = static_cast<time_t>(-1);

  while (!pieceManager_->isComplete()) {
    const time_t now = std::time(nullptr);
    const double elapsed = std::difftime(now, last_peer_query);

    const bool should_query_tracker = last_peer_query == -1 ||
                                      elapsed >= PEER_QUERY_INTERVAL ||
                                      queue_.is_empty();

    if (!should_query_tracker) {
      continue;
    }

    PeerRetriever retriever(peerId_, announce_url, info_hash, PORT, file_size);

    const auto peers =
        retriever.retrievePeers(pieceManager_->bytesDownloaded());

    last_peer_query = now;

    if (!peers.empty()) {
      queue_.clear();
      for (auto* peer : peers) {
        queue_.push_back(peer);
      }
    }
  }

  terminate();

  Logger::log("Download completed!");
  Logger::log(fmt::format("Torrent file '{}' downloaded.", torrentFile));
}

void TorrentClient::terminate() {
  // Signal all worker threads to stop by pushing dummy peers
  for (int i = 0; i < threadNum_; ++i) {
    auto dummy_peer = std::make_unique<Peer>();
    dummy_peer->ip = "0.0.0.0";
    dummy_peer->port = 0;
    // TODO(slim): dont use raw peer pointer
    queue_.push_back(dummy_peer.release());
  }

  // Stop all active connections
  for (auto* connection : connections_) {
    connection->stop();
  }

  // Join all threads
  for (auto& thread : threadPool_) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  // Clean up thread pool
  threadPool_.clear();
}
