#include "TorrentClient.h"

#include <bencode/bencoding.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <random>
#include <thread>

#include "PeerConnection.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "TorrentFileParser.h"

#define PORT 8080
#define PEER_QUERY_INTERVAL 60  // 1 minute

TorrentClient::TorrentClient(
    std::shared_ptr<Logger> logger, std::shared_ptr<TorrentState> torrentState,
    std::shared_ptr<PieceManager> pieceManager,
    std::shared_ptr<TorrentFileParser> torrentFileParser, int threadNum,
    std::string logFilePath)
    : logger(),
      torrentState(torrentState),
      pieceManager(pieceManager),
      torrentFileParser(torrentFileParser),
      threadNum(threadNum),
      peerId("-UT2021-"),
      queue(),
      threadPool(),
      connections()

{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 9);

  if (!this->torrentState) {
    logger->log("torrentState is null!");
    throw std::runtime_error("torrentState is null");
  }
  if (!this->torrentFileParser) {
    logger->log("torrentFileParser is null!");
    throw std::runtime_error("torrentFileParser is null");
  }
  if (!this->pieceManager) {
    logger->log("pieceManager is null!");
    throw std::runtime_error("pieceManager is null");
  }

  for (int i = 0; i < 12; ++i) {
    peerId += std::to_string(distrib(gen));
  }
}

/**
 * Ensures that all resources are freed when TorrentClient is destroyed
 */
TorrentClient::~TorrentClient() = default;

void TorrentClient::start(const std::string& downloadDirectory) {
  const std::string info_hash = torrentFileParser->getInfoHash();
  const std::string filename = torrentFileParser->getFileName().value();

  auto state = torrentState->getState(info_hash);
  if (!state) {
    Logger::log("No state found for this infoHash.");
    return;
  }

  if (state->id == info_hash) {
    Logger::log("Torrent already downloaded\n");
    return;
  }

  downloadFile(downloadDirectory);

  torrentState->storeState(info_hash, filename);
}

void TorrentClient::downloadFile(const std::string& torrentFile) {
  std::string announce_url = torrentFileParser->getAnnounce().value();

  int64_t file_size = torrentFileParser->getFileSize().value();
  const std::string info_hash = torrentFileParser->getInfoHash();
  std::string filename = torrentFileParser->getFileName().value();

  // Adds threads to the thread pool
  for (int i = 0; i < threadNum; i++) {
    PeerConnection connection(queue, peerId, info_hash, pieceManager);

    connections.push_back(&connection);

    std::thread thread(&PeerConnection::start, &connection);
    threadPool.push_back(std::move(thread));
  }

  auto last_peer_query = static_cast<time_t>(-1);

  bool is_download_completed = false;

  while (!is_download_completed) {
    is_download_completed = pieceManager->isComplete();

    time_t current_time = std::time(nullptr);
    auto diff = std::difftime(current_time, last_peer_query);
    // Retrieve peers from the tracker after a certain time interval or
    // whenever the queue is empty
    if (last_peer_query == -1 || diff >= PEER_QUERY_INTERVAL || queue.empty()) {
      PeerRetriever peer_retriever(logger, peerId, announce_url, info_hash,
                                   PORT, file_size);
      std::vector<std::unique_ptr<Peer>> peers =
          peer_retriever.retrievePeers(pieceManager->bytesDownloaded());
      last_peer_query = current_time;

      if (!peers.empty()) {
        queue.clear();
        for (auto& peer : peers) {
          queue.push_back(std::move(peer));
        }
      }
    }
  }

  terminate();

  if (is_download_completed) {
    Logger::log("Download completed!");
    Logger::log(fmt::format("Torrent File {} Downloaded", torrentFile));
  }
}

/**
 * Terminates the download and cleans up all the resources
 */
void TorrentClient::terminate() {
  // Pushes dummy Peers into the queue so that
  // the waiting threads can terminate
  for (int i = 0; i < threadNum; i++) {
    std::unique_ptr<Peer> dummy_peer =
        std::make_unique<Peer>(Peer{.ip = "0.0.0.0", .port = 0});

    queue.push_back(std::move(dummy_peer));
  }
  for (auto* connection : connections) connection->stop();

  for (std::thread& thread : threadPool) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  threadPool.clear();
}
