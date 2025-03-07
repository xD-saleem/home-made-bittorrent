#include "PeerConnection.h"

#include <fmt/base.h>
#include <fmt/core.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
#include <tl/expected.hpp>
#include <utility>

#include "BitTorrentMessage.h"
#include "Piece.h"
#include "connect.h"
#include "utils.h"

#define INFO_HASH_STARTING_POS 28
#define PEER_ID_STARTING_POS 48
#define HASH_LEN 20
#define DUMMY_PEER_IP "0.0.0.0"

/**
 * Constructor of the class PeerConnection.
 * @param queue: the thread-safe queue that contains the available peers.
 * @param clientId: the peer ID of this C++ BitTorrent client. Generated in the
 * TorrentClient class.
 * @param infoHash: info hash of the Torrent file.
 * @param pieceManager: pointer to the PieceManager.
 */
PeerConnection::PeerConnection(SharedQueue<Peer *> *queue, std::string clientId,
                               std::string infoHash,
                               std::shared_ptr<PieceManager> pieceManager)
    : queue(queue),
      clientId(std::move(clientId)),
      infoHash(std::move(infoHash)),
      pieceManager(pieceManager) {}

/**
 * Destructor of the PeerConnection class. Closes the established TCP connection
 * with the peer on object destruction.
 */
PeerConnection::~PeerConnection() { closeSock(); }

tl::expected<void, PeerConnectionError> PeerConnection::start() {
  // TODO put this in a control class
  std::string path = "isPaused.txt";

  while (!(terminated || pieceManager->isComplete())) {
    peer = queue->pop_front();
    // Terminates the thread if it has received a dummy Peer
    if (peer->ip == DUMMY_PEER_IP) {
      return {};
    }

    try {
      // Establishes connection with the peer, and lets it know
      // that we are interested.
      if (establishNewConnection()) {
        while (!pieceManager->isComplete()) {
          bool isPaused = std::filesystem::exists(path);
          if (isPaused) {
            fmt::print("Peer connection is paused\n");
          }
          BitTorrentMessage message = receiveMessage();
          if (message.getMessageId() > 10)
            return tl::make_unexpected(PeerConnectionError{
                "Received invalid message Id from peer " + peerId});

          switch (message.getMessageId()) {
            case choke:
              choked = true;
              break;

            case unchoke:
              choked = false;
              break;

            case piece: {
              requestPending = false;
              std::string payload = message.getPayload();
              int index = bytesToInt(payload.substr(0, 4));
              int begin = bytesToInt(payload.substr(4, 4));
              std::string blockData = payload.substr(8);
              pieceManager->blockReceived(peerId, index, begin, blockData);
              break;
            }
            case have: {
              std::string payload = message.getPayload();
              int pieceIndex = bytesToInt(payload);
              pieceManager->updatePeer(peerId, pieceIndex);
              break;
            }

            default:
              break;
          }
          if (!choked) {
            if (!requestPending) {
              requestPiece();
            }
          }
        }
      }
    } catch (std::exception &e) {
      closeSock();
    }
  }
  return {};
}

void PeerConnection::stop() { terminated = true; }

tl::expected<void, PeerConnectionError> PeerConnection::performHandshake() {
  // Connects to the peer
  try {
    auto sockOption = createConnection(peer->ip, peer->port);
    sock = sockOption.value();
  } catch (std::runtime_error &e) {
    return tl::make_unexpected(PeerConnectionError{e.what()});
  }

  // Send the handshake message to the peer
  std::string handshakeMessage = createHandshakeMessage();
  sendData(sock, handshakeMessage);

  // Receive the reply from the peer
  auto replyOption = receiveData(sock, handshakeMessage.length());

  auto reply = replyOption.value();
  if (reply.empty()) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive handshake from peer: FAILED [No response from peer]"});
  }
  peerId = reply.substr(PEER_ID_STARTING_POS, HASH_LEN);

  std::string receivedInfoHash = reply.substr(INFO_HASH_STARTING_POS, HASH_LEN);
  if ((receivedInfoHash == infoHash) != 0) {
    return tl::make_unexpected(
        PeerConnectionError{"Perform handshake with peer " + peer->ip +
                            ": FAILED [Received mismatching info hash]"});
  }

  return {};
}

tl::expected<void, PeerConnectionError> PeerConnection::receiveBitField() {
  // Receive BitField from the peer
  BitTorrentMessage message = receiveMessage();
  if (message.getMessageId() != bitField) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive BitField from peer: FAILED [Wrong message ID]"});
  }
  peerBitField = message.getPayload();

  // Informs the PieceManager of the BitField received
  pieceManager->addPeer(peerId, peerBitField);
  return {};
}

void PeerConnection::requestPiece() {
  Block *block = pieceManager->nextRequest(peerId);

  if (!block) return;

  int payloadLength = 12;
  char temp[payloadLength];
  // Needs to convert little-endian to big-endian
  uint32_t index = htonl(block->piece);
  uint32_t offset = htonl(block->offset);
  uint32_t length = htonl(block->length);
  std::memcpy(temp, &index, sizeof(int));
  std::memcpy(temp + 4, &offset, sizeof(int));
  std::memcpy(temp + 8, &length, sizeof(int));
  std::string payload;
  for (int i = 0; i < payloadLength; i++) payload += (char)temp[i];

  std::stringstream info;
  info << "Sending Request message to peer " << peer->ip << " ";
  info << "[Piece: " << std::to_string(block->piece) << " ";
  info << "Offset: " << std::to_string(block->offset) << " ";
  info << "Length: " << std::to_string(block->length) << "]";
  std::string requestMessage = BitTorrentMessage(request, payload).toString();
  sendData(sock, requestMessage);
  requestPending = true;
}

tl::expected<void, PeerConnectionError> PeerConnection::sendInterested() {
  std::string interestedMessage = BitTorrentMessage(interested).toString();
  sendData(sock, interestedMessage);
  return {};
}

tl::expected<void, PeerConnectionError> PeerConnection::receiveUnchoke() {
  BitTorrentMessage message = receiveMessage();
  if (message.getMessageId() != unchoke) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive Unchoke from peer: FAILED [Wrong message ID]"});
  }
  choked = false;
  return {};
}

tl::expected<void, PeerConnectionError>
PeerConnection::establishNewConnection() {
  if (auto handshakeResult = performHandshake(); !handshakeResult) {
    return tl::unexpected(handshakeResult.error());
  }

  if (auto bitfieldResult = receiveBitField(); !bitfieldResult) {
    return tl::unexpected(bitfieldResult.error());
  }

  sendInterested();

  return {};
}

std::string PeerConnection::createHandshakeMessage() {
  static constexpr char PROTOCOL_NAME[] = "BitTorrent protocol";
  static constexpr size_t RESERVED_BYTES = 8;

  std::ostringstream buffer;

  // Append protocol length and protocol name
  buffer.put(static_cast<char>(sizeof(PROTOCOL_NAME) - 1));
  buffer.write(PROTOCOL_NAME, sizeof(PROTOCOL_NAME) - 1);

  // Append 8 reserved bytes (all set to zero)
  buffer.write("\0\0\0\0\0\0\0\0", RESERVED_BYTES);

  // Append info hash and client ID
  buffer << hexDecode(infoHash) << clientId;

  assert(buffer.str().size() == (sizeof(PROTOCOL_NAME) - 1) + 49);
  return buffer.str();
}

BitTorrentMessage PeerConnection::receiveMessage(int bufferSize) const {
  auto replyOption = receiveData(sock, 0);
  std::string reply = replyOption.value();
  if (reply.empty()) {
    return BitTorrentMessage(keepAlive);
  }

  auto messageId = (uint8_t)reply[0];
  std::string payload = reply.substr(1);
  return BitTorrentMessage(messageId, payload);
}

const std::string &PeerConnection::getPeerId() const { return peerId; }

void PeerConnection::closeSock() {
  if (!sock) {
    return;
  }

  close(sock);
  sock = {};

  requestPending = false;

  if (!peerBitField.empty()) {
    peerBitField.clear();
    if (pieceManager) {
      pieceManager->removePeer(peerId);
    }
  }
}
