
#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <memory>
#include <string>

#include "Logger.h"
#include "PeerConnection.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "SharedQueue.h"
#include "TorrentFileParser.h"
#include "TorrentState.h"

struct TorrentClientError {
  std::string message;
};

class TorrentClient {
 private:
  // Deps
  std::shared_ptr<Logger> logger;
  std::shared_ptr<TorrentState> torrentState;
  std::shared_ptr<PieceManager> pieceManager;
  std::shared_ptr<TorrentFileParser> torrentFileParser;

  const int threadNum = 5;
  std::string peerId;
  SharedQueue<std::unique_ptr<Peer>> queue;
  std::vector<std::thread> threadPool;
  std::vector<std::shared_ptr<PeerConnection>> connections;

 public:
  // Constructor that accepts a shared_ptr to TorrentState
  explicit TorrentClient(std::shared_ptr<Logger> logger,
                         std::shared_ptr<TorrentState> torrentState,
                         std::shared_ptr<PieceManager> pieceManager,
                         std::shared_ptr<TorrentFileParser> torrentFileParser,
                         int threadNum = 5,
                         std::string logFilePath = "logs/client.log");
  // Destructor
  ~TorrentClient();

  // Method to terminate the client (assumed to stop all threads/connections)
  void terminate();

  // Main download method
  void start(const std::string& downloadDirectory);

  // Method to download file from torrent
  void downloadFile(const std::string& downloadDirectory);

  // Method to seed file
  void seedFile(const std::string& downloadDirectory);
};

#endif  // BITTORRENTCLIENT_TORRENTCLIENT_H
