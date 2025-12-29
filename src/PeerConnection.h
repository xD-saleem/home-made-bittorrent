
#ifndef BITTORRENTCLIENT_PEERCONNECTION_H
#define BITTORRENTCLIENT_PEERCONNECTION_H

#include <memory>

#include "BitTorrentMessage.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "Queue.h"

using byte = unsigned char;

struct PeerConnectionError {
  std::string message;
};

class PeerConnection {
 private:
  int sock_{};

  std::unique_ptr<Queue<std::unique_ptr<Peer>>> queue_;
  bool choked_ = true;
  bool terminated_ = false;
  bool requestPending_ = false;

  const std::string clientId_;
  const std::string infoHash_;

  std::unique_ptr<Peer> peer_;
  std::string peerBitField_;
  std::string peerId_;
  std::shared_ptr<PieceManager> pieceManager_;

  std::string createHandshakeMessage();
  tl::expected<void, PeerConnectionError> performHandshake();
  tl::expected<void, PeerConnectionError> receiveBitField();
  tl::expected<void, PeerConnectionError> sendBitField();
  tl::expected<void, PeerConnectionError> sendInterested();
  tl::expected<void, PeerConnectionError> receiveUnchoke();
  void requestPiece();
  void closeSock();
  tl::expected<void, PeerConnectionError> establishNewConnection();
  BitTorrentMessage receiveMessage() const;
  BitTorrentMessage sendMessage(int bufferSize = 0) const;

  // seed
  void sendSeed();
  void sendPiece();

 public:
  const std::string& getPeerId() const;

  explicit PeerConnection(std::unique_ptr<Queue<std::unique_ptr<Peer>>> queue,
                          std::string clientId, std::string infoHash,
                          std::shared_ptr<PieceManager> pm);
  ~PeerConnection();
  tl::expected<void, PeerConnectionError> start();
  void stop();
};

#endif  // BITTORRENTCLIENT_PEERCONNECTION_H
