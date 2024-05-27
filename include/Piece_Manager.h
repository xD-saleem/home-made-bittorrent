
#include <ctime>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Piece.h"
#include "Torrent_Parser.h"

#ifndef BITTORRENTCLIENT_PIECE_MANAGER
#define BITTORRENTCLIENT_PIECE_MANAGER

struct PendingRequest {
  Block* block;
  time_t timestamp;
};

class Piece_Manager {
 private:
  std::map<std::string, std::string> peers;
  std::vector<Piece*> missingPieces;
  std::vector<Piece*> ongoingPieces;
  std::vector<Piece*> havePieces;
  std::vector<PendingRequest*> pendingRequests;
  std::ofstream downloadedFile;
  const long pieceLength;
  const Torrent_Parser& fileParser;
  int piecesDownloadedInInterval = 0;
  time_t startingTime;
  int totalPieces{};

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
  const int maximumConnections;
  explicit Piece_Manager(const Torrent_Parser& fileParser,
                         const std::string& downloadPath,
                         const int maximumConnections);
  ~Piece_Manager();
  bool isComplete();
  void blockReceived(std::string peerId, int pieceIndex, int blockOffset,
                     std::string data);
  void addPeer(const std::string& peerId, std::string bitField);
  void removePeer(const std::string& peerId);
  void updatePeer(const std::string& peerId, int index);

  std::map<std::string, std::string> getPeers();

  unsigned long bytesDownloaded();
  Block* nextRequest(std::string peerId);
};

#endif  // BITTORRENTCLIENT_PIECE_MANAGER_H
