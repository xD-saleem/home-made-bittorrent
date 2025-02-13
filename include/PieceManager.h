
#ifndef BITTORRENTCLIENT_PIECEMANAGER_H
#define BITTORRENTCLIENT_PIECEMANAGER_H

#include <ctime>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>

#include "Piece.h"
#include "TorrentFileParser.h"

struct PendingRequest {
  Block *block;
  time_t timestamp;
};

struct PieceManagerError {
  std::string message;
};

class PieceManager {
private:
  std::map<std::string, std::string> peers;
  std::vector<Piece *> missingPieces;
  std::vector<Piece *> ongoingPieces;
  std::vector<PendingRequest *> pendingRequests;
  std::ofstream downloadedFile;
  // std::thread& progressTrackerThread;
  const long pieceLength;
  std::shared_ptr<TorrentFileParser> fileParser;
  const int maximumConnections;
  int piecesDownloadedInInterval = 0;
  time_t startingTime;

  // Uses a lock to prevent race condition
  std::mutex lock;

  std::vector<Piece *> initiatePieces();
  Block *expiredRequest(std::string peerId);
  Block *nextOngoing(std::string peerId);
  Piece *getRarestPiece(std::string peerId);
  void write(Piece *piece);
  void displayProgressBar();
  void trackProgress();

public:
  explicit PieceManager(std::shared_ptr<TorrentFileParser> fileParser,
                        const std::string &downloadPath,
                        int maximumConnections);
  ~PieceManager();
  bool isComplete();
  tl::expected<void, PieceManagerError> blockReceived(std::string peerId,
                                                      int pieceIndex,
                                                      int blockOffset,
                                                      std::string data);
  void addPeer(const std::string &peerId, std::string bitField);
  tl::expected<void, PieceManagerError> removePeer(const std::string &peerId);
  tl::expected<void, PieceManagerError> updatePeer(const std::string &peerId,
                                                   int index);

  std::vector<Piece *> getPieces();
  unsigned long bytesDownloaded();
  size_t totalPieces{};
  Block *nextRequest(std::string peerId);
  std::vector<Piece *> havePieces;
};

#endif // BITTORRENTCLIENT_PIECEMANAGER_H
