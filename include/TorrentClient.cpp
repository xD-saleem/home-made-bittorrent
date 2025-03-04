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
#define PEER_QUERY_INTERVAL 60 // 1 minute

TorrentClient::TorrentClient(
    std::shared_ptr<Logger> logger, std::shared_ptr<TorrentState> torrentState,
    std::shared_ptr<PieceManager> pieceManager,
    std::shared_ptr<TorrentFileParser> torrentFileParser, int threadNum,
    std::string logFilePath)
    : logger(), torrentState(torrentState), pieceManager(pieceManager),
      torrentFileParser(torrentFileParser), threadNum(threadNum),
      peerId("-UT2021-"), queue(), threadPool(), connections()

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

void TorrentClient::start(const std::string &downloadDirectory) {
  const std::string infoHash = torrentFileParser->getInfoHash();
  const std::string filename = torrentFileParser->getFileName().value();

  auto state = torrentState->getState(infoHash);
  if (!state) {
    logger->log("No state found for this infoHash.");
    return;
  }

  if (state->id == infoHash) {
    logger->log("Torrent already downloaded\n");
    return;
  }

  downloadFile(downloadDirectory);

  torrentState->storeState(infoHash, filename);
}

void TorrentClient::downloadFile(const std::string &downloadDirectory) {
  std::string announceUrl = torrentFileParser->getAnnounce().value();

  long fileSize = torrentFileParser->getFileSize().value();
  const std::string infoHash = torrentFileParser->getInfoHash();
  std::string filename = torrentFileParser->getFileName().value();

  std::string downloadPath = downloadDirectory + filename;

  // Adds threads to the thread pool
  for (int i = 0; i < threadNum; i++) {
    PeerConnection connection(&queue, peerId, infoHash, pieceManager);
    connections.push_back(&connection);
    std::thread thread(&PeerConnection::start, connection);
    threadPool.push_back(std::move(thread));
  }

  auto lastPeerQuery = (time_t)(-1);

  logger->log(fmt::format("Downloading file to {}\n", downloadPath));

  bool isDownloadCompleted = false;

  while (!isDownloadCompleted) {
    isDownloadCompleted = pieceManager->isComplete();

    time_t currentTime = std::time(nullptr);
    auto diff = std::difftime(currentTime, lastPeerQuery);
    // Retrieve peers from the tracker after a certain time interval or
    // whenever the queue is empty
    if (lastPeerQuery == -1 || diff >= PEER_QUERY_INTERVAL || queue.empty()) {
      PeerRetriever peerRetriever(logger, peerId, announceUrl, infoHash, PORT,
                                  fileSize);
      std::vector<Peer *> peers =
          peerRetriever.retrievePeers(pieceManager->bytesDownloaded());
      lastPeerQuery = currentTime;

      if (!peers.empty()) {
        queue.clear();
        for (auto peer : peers) {
          queue.push_back(peer);
        }
      }
    }
  }

  terminate();

  if (isDownloadCompleted) {
    logger->log("Download completed!");
    logger->log(fmt::format("File downloaded to {}", downloadPath));
  }
}

/**
 * Terminates the download and cleans up all the resources
 */
void TorrentClient::terminate() {
  // Pushes dummy Peers into the queue so that
  // the waiting threads can terminate
  for (int i = 0; i < threadNum; i++) {
    Peer *dummyPeer = new Peer{"0.0.0.0", 0};
    queue.push_back(dummyPeer);
  }
  for (auto connection : connections)
    connection->stop();

  for (std::thread &thread : threadPool) {
    if (thread.joinable()) {
      thread.join();
    }
  }

  threadPool.clear();
}
