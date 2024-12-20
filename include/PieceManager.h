
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
  Block* block;
  time_t timestamp;
};

struct PieceManagerError {
  std::string message;
};

/**
 * Responsible for keeping track of all the available pieces
 * from the peers. Implementation is based on the Python code
 * in this repository:
 * https://github.com/eliasson/pieces/
 */
class PieceManager {
 private:
  std::map<std::string, std::string> peers;
  std::vector<Piece*> missingPieces;
  std::vector<Piece*> ongoingPieces;
  std::vector<Piece*> havePieces;
  std::vector<PendingRequest*> pendingRequests;
  std::ofstream downloadedFile;
  // std::thread& progressTrackerThread;
  const long pieceLength;
  const TorrentFileParser& fileParser;
  const int maximumConnections;
  int piecesDownloadedInInterval = 0;
  time_t startingTime;
  size_t totalPieces{};

  // Uses a lock to prevent race condition
  std::mutex lock;

  std::vector<Piece*> initiatePieces();
  Block* expiredRequest(std::string peerId);
  Block* nextOngoing(std::string peerId);
  Piece* getRarestPiece(std::string peerId);
  void write(Piece* piece);
  void displayProgressBar();
  void trackProgress();

 public:
  explicit PieceManager(const TorrentFileParser& fileParser,
                        const std::string& downloadPath,
                        int maximumConnections);
  ~PieceManager();
  bool isComplete();
  tl::expected<void, PieceManagerError> blockReceived(std::string peerId,
                                                      int pieceIndex,
                                                      int blockOffset,
                                                      std::string data);
  void addPeer(const std::string& peerId, std::string bitField);
  tl::expected<void, PieceManagerError> removePeer(const std::string& peerId);
  tl::expected<void, PieceManagerError> updatePeer(const std::string& peerId,
                                                   int index);

  unsigned long bytesDownloaded();
  Block* nextRequest(std::string peerId);
};

#endif  // BITTORRENTCLIENT_PIECEMANAGER_H
