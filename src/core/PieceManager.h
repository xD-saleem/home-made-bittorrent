
#ifndef BITTORRENTCLIENT_PIECEMANAGER_H
#define BITTORRENTCLIENT_PIECEMANAGER_H

#include <cstdint>
#include <ctime>
#include <mutex>
#include <vector>

#include "core/PeerRegistry.h"
#include "core/Piece.h"
#include "infra/DiskManager.h"
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
  std::vector<std::unique_ptr<Piece>> missingPieces_;
  std::vector<std::unique_ptr<Piece>> ongoingPieces_;

  std::vector<std::unique_ptr<PendingRequest>> pendingRequests_;

  std::shared_ptr<TorrentFileParser> fileParser_;
  std::shared_ptr<PeerRegistry> peerRegistry_;
  std::shared_ptr<DiskManager> diskManager_;

  const int64_t pieceLength_;
  size_t total_pieces_{};
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
                        const std::shared_ptr<PeerRegistry>& peerRegistry,
                        const std::shared_ptr<DiskManager>& diskManager,
                        const std::string& downloadPath,
                        int maximumConnections);
  ~PieceManager() = default;
  bool isComplete();
  tl::expected<void, PieceManagerError> blockReceived(int pieceIndex,
                                                      int blockOffset,
                                                      const std::string& data);

  std::vector<Piece*> getPieces();
  uint64_t bytesDownloaded();
  Block* nextRequest(std::string peerId);
  std::vector<Piece*> havePieces;
};

#endif  // BITTORRENTCLIENT_PIECEMANAGER_H
