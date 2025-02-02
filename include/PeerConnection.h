
#ifndef BITTORRENTCLIENT_PEERCONNECTION_H
#define BITTORRENTCLIENT_PEERCONNECTION_H

#include "BitTorrentMessage.h"
#include "PeerRetriever.h"
#include "PieceManager.h"
#include "SharedQueue.h"

using byte = unsigned char;

struct PeerConnectionError {
  std::string message;
};

class PeerConnection {
private:
  int sock{};
  SharedQueue<Peer *> *queue;
  bool choked = true;
  bool terminated = false;
  bool requestPending = false;
  const std::string clientId;
  const std::string infoHash;
  Peer *peer;
  std::string peerBitField;
  std::string peerId;
  PieceManager *pieceManager;

  std::string createHandshakeMessage();
  tl::expected<void, PeerConnectionError> performHandshake();
  tl::expected<void, PeerConnectionError> receiveBitField();
  tl::expected<void, PeerConnectionError> sendBitField();
  void sendInterested();
  tl::expected<void, PeerConnectionError> receiveUnchoke();
  void requestPiece();
  void closeSock();
  bool establishNewConnection();
  bool establishSeedNewConnection();
  BitTorrentMessage receiveMessage(int bufferSize = 0) const;
  BitTorrentMessage sendMessage(int bufferSize = 0) const;

  // seed
  void sendSeed();
  void sendPiece();

public:
  const std::string &getPeerId() const;

  explicit PeerConnection(SharedQueue<Peer *> *queue, std::string clientId,
                          std::string infoHash, PieceManager *pieceManager);
  ~PeerConnection();
  tl::expected<void, PeerConnectionError> start();

  std::string XXX();
  tl::expected<void, PeerConnectionError> seed();
  void stop();
};

#endif // BITTORRENTCLIENT_PEERCONNECTION_H
