
#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <memory>
#include <string>

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
  // Dependencies
  std::shared_ptr<TorrentState> torrentState; // Shared pointer to TorrentState

  // Variables
  const int threadNum;
  std::string peerId;
  SharedQueue<Peer *> queue;
  std::vector<std::thread> threadPool;
  std::vector<PeerConnection *> connections;
  std::shared_ptr<PieceManager> pieceManager;
  std::shared_ptr<TorrentFileParser> torrentFileParser;

public:
  // Constructor that accepts a shared_ptr to TorrentState
  explicit TorrentClient(std::shared_ptr<TorrentState> torrentState,
                         std::shared_ptr<PieceManager> pieceManager,
                         std::shared_ptr<TorrentFileParser> torrentFileParser,
                         int threadNum = 5, bool enableLogging = true,
                         std::string logFilePath = "logs/client.log");
  // Destructor
  ~TorrentClient();

  // Method to terminate the client (assumed to stop all threads/connections)
  void terminate();

  // Main download method
  void start(const std::string &downloadDirectory);

  // Method to download file from torrent
  void downloadFile(const std::string &downloadDirectory);

  // Method to seed file
  void seedFile(const std::string &downloadDirectory);
};

#endif // BITTORRENTCLIENT_TORRENTCLIENT_H
