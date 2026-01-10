
#ifndef BITTORRENTCLIENT_PIECEMANAGER_H
#define BITTORRENTCLIENT_PIECEMANAGER_H

#include <cstdint>
#include <ctime>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>

#include "core/Piece.h"
#include "utils/TorrentFileParser.h"

struct PendingRequest {
  Block* block;
  time_t timestamp;
};

struct PieceManagerError {
  std::string message;
};

class PieceManager {
 private:
  std::map<std::string, std::string> peers_;
  std::vector<std::unique_ptr<Piece>> missingPieces_;
  std::vector<std::unique_ptr<Piece>> ongoingPieces_;
  std::vector<PendingRequest*> pendingRequests_;
  std::ofstream downloadedFile_;
  const int64_t pieceLength_;
  std::shared_ptr<TorrentFileParser> fileParser_;
  const int maximumConnections_;
  int piecesDownloadedInInterval_ = 0;
  time_t startingTime_;

  // Uses a lock to prevent race condition
  std::mutex lock_;

  std::vector<std::unique_ptr<Piece>> initiatePieces();
  Block* expiredRequest(std::string peerId);
  Block* nextOngoing(std::string peerId);
  Piece* getRarestPiece(const std::string& peerId);
  void write(Piece* piece);
  void displayProgressBar();
  void trackProgress();

 public:
  explicit PieceManager(const std::shared_ptr<TorrentFileParser>& fileParser,
                        const std::string& downloadPath,
                        int maximumConnections);
  ~PieceManager();
  bool isComplete();
  tl::expected<void, PieceManagerError> blockReceived(int pieceIndex,
                                                      int blockOffset,
                                                      std::string data);
  void addPeer(const std::string& peerId, std::string bitField);
  tl::expected<void, PieceManagerError> removePeer(const std::string& peerId);
  tl::expected<void, PieceManagerError> updatePeer(const std::string& peerId,
                                                   int index);

  std::vector<Piece*> getPieces();
  uint64_t bytesDownloaded();
  size_t total_pieces{};
  Block* nextRequest(std::string peerId);
  std::vector<Piece*> havePieces;
};

#endif  // BITTORRENTCLIENT_PIECEMANAGER_H
