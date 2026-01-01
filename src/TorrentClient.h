
#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <algorithm>
#include <memory>
#include <string>

#include "PeerConnection.h"
#include "PieceManager.h"
#include "Queue.h"
#include "TorrentFileParser.h"
#include "TorrentState.h"

struct TorrentClientError {
  std::string message;
};

class TorrentClient {
 private:
  // Deps
  std::shared_ptr<TorrentState> torrentState_;
  std::shared_ptr<PieceManager> pieceManager_;
  std::shared_ptr<TorrentFileParser> torrentFileParser_;

  const int threadNum_ = 5;
  std::string peerId_;
  std::shared_ptr<Queue<std::unique_ptr<Peer>>> queue_;
  std::vector<std::thread> threadPool_;
  std::vector<std::unique_ptr<PeerConnection>> connections_;

 public:
  // Constructor that accepts a shared_ptr to TorrentState
  explicit TorrentClient(std::shared_ptr<Queue<std::unique_ptr<Peer>>> queue,
                         std::shared_ptr<TorrentState> torrentState,
                         std::shared_ptr<PieceManager> pieceManager,
                         std::shared_ptr<TorrentFileParser> torrentFileParser,
                         int threadNum = 5);
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
