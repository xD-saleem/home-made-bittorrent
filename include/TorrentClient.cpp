#include "TorrentClient.h"

#include <bencode/bencoding.h>
#include <fmt/core.h>

#include <filesystem>
#include <iostream>
#include <loguru/loguru.hpp>
#include <random>
#include <thread>

#include "PeerConnection.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "TorrentFileParser.h"

#define PORT 8080
#define PEER_QUERY_INTERVAL 60  // 1 minute

TorrentClient::TorrentClient(const int threadNum, bool enableLogging,
                             std::string logFilePath)
    : threadNum(threadNum) {
  peerId = "-UT2021-";

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 9);
  for (int i = 0; i < 12; i++) peerId += std::to_string(distrib(gen));

  if (enableLogging) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::g_flush_interval_ms = 100;
    loguru::add_file(logFilePath.c_str(), loguru::Truncate,
                     loguru::Verbosity_MAX);
  } else {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
  }
}

/**
 * Ensures that all resources are freed when TorrentClient is destroyed
 */
TorrentClient::~TorrentClient() = default;

void TorrentClient::downloadFile(const std::string& torrentFilePath,
                                 const std::string& downloadDirectory) {
  // Parse Torrent file
  fmt::print("Parsing torrent file {}\n", torrentFilePath);

  TorrentFileParser torrentFileParser(torrentFilePath);
  std::string announceUrl = torrentFileParser.getAnnounce().value();

  long fileSize = torrentFileParser.getFileSize().value();
  const std::string infoHash = torrentFileParser.getInfoHash();

  std::string filename = torrentFileParser.getFileName().value();
  std::string downloadPath = downloadDirectory + filename;

  PieceManager pieceManager(torrentFileParser, downloadPath, threadNum);

  // Adds threads to the thread pool
  for (int i = 0; i < threadNum; i++) {
    PeerConnection connection(&queue, peerId, infoHash, &pieceManager);
    connections.push_back(&connection);
    std::thread thread(&PeerConnection::start, connection);
    threadPool.push_back(std::move(thread));
  }

  auto lastPeerQuery = (time_t)(-1);

  fmt::print("Downloading file to {}\n", downloadPath);

  while (true) {
    if (pieceManager.isComplete()) {
      break;
    }

    time_t currentTime = std::time(nullptr);
    auto diff = std::difftime(currentTime, lastPeerQuery);
    // Retrieve peers from the tracker after a certain time interval or
    // whenever the queue is empty
    if (lastPeerQuery == -1 || diff >= PEER_QUERY_INTERVAL || queue.empty()) {
      PeerRetriever peerRetriever(peerId, announceUrl, infoHash, PORT,
                                  fileSize);
      std::vector<Peer*> peers =
          peerRetriever.retrievePeers(pieceManager.bytesDownloaded());
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

  if (pieceManager.isComplete()) {
    std::cout << "Download completed!" << std::endl;
    std::cout << "File downloaded to " << downloadPath << std::endl;
  }
}

/**
 * Terminates the download and cleans up all the resources
 */
void TorrentClient::terminate() {
  // Pushes dummy Peers into the queue so that
  // the waiting threads can terminate
  for (int i = 0; i < threadNum; i++) {
    Peer* dummyPeer = new Peer{"0.0.0.0", 0};
    queue.push_back(dummyPeer);
  }
  for (auto connection : connections) connection->stop();

  for (std::thread& thread : threadPool) {
    if (thread.joinable()) thread.join();
  }

  threadPool.clear();
}
