#include "TorrentClient.h"

#include <bencode/bencoding.h>
#include <fmt/core.h>

#include <iostream>
#include <random>
#include <thread>

#include "PeerConnection.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "TorrentFileParser.h"

#define PORT 8080
#define PEER_QUERY_INTERVAL 60 // 1 minute

TorrentClient::TorrentClient(std::shared_ptr<TorrentState> torrentState,
                             int threadNum, bool enableLogging,
                             std::string logFilePath)
    : torrentState(std::move(torrentState)), threadNum(threadNum) {
  peerId = "-UT2021-";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 9);

  for (int i = 0; i < 12; ++i) {
    peerId += std::to_string(distrib(gen));
  }

  // Enable logging if required
  if (enableLogging) {
    // loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    // loguru::g_flush_interval_ms = 100;
    // loguru::add_file(logFilePath.c_str(), loguru::Truncate,
    // loguru::Verbosity_MAX);
  } else {
    // loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
  }
}

/**
 * Ensures that all resources are freed when TorrentClient is destroyed
 */
TorrentClient::~TorrentClient() = default;

void TorrentClient::start(const std::string &torrentFilePath,
                          const std::string &downloadDirectory) {
  std::cout << "Parsing Torrent file " + torrentFilePath + "..." << std::endl;

  TorrentFileParser torrentFileParser(torrentFilePath);

  const std::string infoHash = torrentFileParser.getInfoHash();
  const std::string filename = torrentFileParser.getFileName().value();

  auto e = torrentState->getState(infoHash);
  if (!e) {
    fmt::print("No state found for this infoHash.\n");
    return;
  }

  if (e->id == infoHash) {
    fmt::print("Torrent already downloaded\n");
    // TODO handle error
    seedFile(torrentFilePath, downloadDirectory);
    return;
  }

  // TODO handle error
  downloadFile(torrentFilePath, downloadDirectory);

  torrentState->storeState(infoHash, filename);
}

void TorrentClient::seedFile(const std::string &torrentFilePath,
                             const std::string &downloadDirectory) {
  fmt::print("Parsing downloaded torrent file {}\n", torrentFilePath);

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
    std::thread thread(&PeerConnection::seed, connection);
    threadPool.push_back(std::move(thread));
  }

  auto lastPeerQuery = (time_t)(-1);

  fmt::print("seeding file to {}\n", downloadPath);

  bool isSeededCompleted = false;

  while (!isSeededCompleted) {
    // isSeededCompleted = pieceManager.isComplete();
    // isSeededCompleted = true;
    //
    time_t currentTime = std::time(nullptr);
    auto diff = std::difftime(currentTime, lastPeerQuery);
    // Retrieve peers from the tracker after a certain time interval or
    // whenever the queue is empty
    if (lastPeerQuery == -1 || diff >= PEER_QUERY_INTERVAL || queue.empty()) {
      PeerRetriever peerRetriever(peerId, announceUrl, infoHash, PORT,
                                  fileSize);
      std::vector<Peer *> peers =
          peerRetriever.retrieveSeedPeers(pieceManager.bytesDownloaded());

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

  if (isSeededCompleted) {
    std::cout << "Seeded completed!" << std::endl;
    std::cout << "File downloaded to " << downloadPath << std::endl;
  }
}

void TorrentClient::downloadFile(const std::string &torrentFilePath,
                                 const std::string &downloadDirectory) {
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

  bool isDownloadCompleted = false;

  while (!isDownloadCompleted) {
    isDownloadCompleted = pieceManager.isComplete();

    time_t currentTime = std::time(nullptr);
    auto diff = std::difftime(currentTime, lastPeerQuery);
    // Retrieve peers from the tracker after a certain time interval or
    // whenever the queue is empty
    if (lastPeerQuery == -1 || diff >= PEER_QUERY_INTERVAL || queue.empty()) {
      PeerRetriever peerRetriever(peerId, announceUrl, infoHash, PORT,
                                  fileSize);
      std::vector<Peer *> peers =
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

  if (isDownloadCompleted) {
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
