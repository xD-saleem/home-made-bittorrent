
#include <memory>

#include "core/PeerRegistry.h"
#include "core/Piece.h"
#include "core/PieceManager.h"

class PieceRepository {
 private:
  // deps
  std::shared_ptr<PeerRegistry> peerRegistry_;
  std::shared_ptr<TorrentFileParser> fileParser_;

  // init
  std::vector<std::unique_ptr<Piece>> missingPieces_;
  std::vector<std::unique_ptr<Piece>> ongoingPieces_;
  std::vector<std::unique_ptr<PendingRequest>> pendingRequests_;
  size_t total_pieces_{};

  // constructor
  explicit PieceRepository(
      const std::shared_ptr<PeerRegistry>& pr,
      const std::shared_ptr<TorrentFileParser>& fileParser);

  // Destructor
  ~PieceRepository() = default;

 public:
  std::vector<std::unique_ptr<Piece>> initiatePieces();
  Piece* acquireRarest(const std::string& peerId);
  // Block* nextBlockForPeer(const std::string peerId&);
  // void markBlockReceived(int piece, int offset, std::string data);
  // void markPieceComplete(Piece*);
  // bool isComplete() const;
};
