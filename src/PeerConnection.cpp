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
#include "connect.h"
#include "utils.h"

#define INFO_HASH_STARTING_POS 28
#define PEER_ID_STARTING_POS 48
#define HASH_LEN 20
#define DUMMY_PEER_IP "0.0.0.0"

/**
 * Constructor of the class PeerConnection.
 * @param queue: the thread-safe queue that contains the available peers.
 * @param clientId: the peer ID of this C++ BitTorrent client. Generated in
 * the TorrentClient class.
 * @param infoHash: info hash of the Torrent file.
 * @param pieceManager: pointer to the PieceManager.
 */
PeerConnection::PeerConnection(SharedQueue<Peer*>* queue, std::string clientId,
                               std::string infoHash,
                               std::shared_ptr<PieceManager> pieceManager)
    : queue_(queue),
      clientId_(std::move(clientId)),
      infoHash_(std::move(infoHash)),
      pieceManager_(std::move(pieceManager)) {}

/**
 * Destructor of the PeerConnection class. Closes the established TCP connection
 * with the peer on object destruction.
 */
PeerConnection::~PeerConnection() { closeSock(); }

tl::expected<void, PeerConnectionError> PeerConnection::start() {
  while (!(terminated_ || pieceManager_->isComplete())) {
    peer_ = queue_->pop_front();
    // Terminates the thread if it has received a dummy Peer
    if (peer_->ip == DUMMY_PEER_IP) {
      return {};
    }

    try {
      // Establishes connection with the peer, and lets it know
      // that we are interested.
      if (establishNewConnection()) {
        while (!pieceManager_->isComplete()) {
          BitTorrentMessage message = receiveMessage();
          if (message.getMessageId() > 10)
            return tl::make_unexpected(PeerConnectionError{
                "Received invalid message Id from peer " + peerId_});

          switch (message.getMessageId()) {
            case kChoke:
              choked_ = true;
              break;

            case kUnchoke:
              choked_ = false;
              break;

            case kPiece: {
              requestPending_ = false;
              std::string payload = message.getPayload();
              int index = utils::bytesToInt(payload.substr(0, 4));
              int begin = utils::bytesToInt(payload.substr(4, 4));
              std::string block_data = payload.substr(8);
              pieceManager_->blockReceived(index, begin, block_data);
              break;
            }
            case kHave: {
              std::string payload = message.getPayload();
              int piece_index = utils::bytesToInt(payload);
              pieceManager_->updatePeer(peerId_, piece_index);
              break;
            }

            default:
              break;
          }
          if (!choked_) {
            if (!requestPending_) {
              requestPiece();
            }
          }
        }
      }
    } catch (std::exception& e) {
      closeSock();
    }
  }
  return {};
}

void PeerConnection::stop() { terminated_ = true; }

tl::expected<void, PeerConnectionError> PeerConnection::performHandshake() {
  // Connects to the peer
  try {
    auto sock_option = createConnection(peer_->ip, peer_->port);
    sock_ = sock_option.value();
  } catch (std::runtime_error& e) {
    return tl::make_unexpected(PeerConnectionError{e.what()});
  }

  // Send the handshake message to the peer
  std::string handshake_message = createHandshakeMessage();
  sendData(sock_, handshake_message);

  // Receive the reply from the peer
  auto reply_option = receiveData(sock_, handshake_message.length());

  auto reply = reply_option.value();
  if (reply.empty()) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive handshake from peer: FAILED [No response from peer]"});
  }
  peerId_ = reply.substr(PEER_ID_STARTING_POS, HASH_LEN);

  std::string received_info_hash =
      reply.substr(INFO_HASH_STARTING_POS, HASH_LEN);
  if (received_info_hash == infoHash_) {
    return tl::make_unexpected(
        PeerConnectionError{"Perform handshake with peer " + peer_->ip +
                            ": FAILED [Received mismatching info hash]"});
  }

  return {};
}

tl::expected<void, PeerConnectionError> PeerConnection::receiveBitField() {
  // Receive BitField from the peer
  BitTorrentMessage message = receiveMessage();
  if (message.getMessageId() != kBitField) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive BitField from peer: FAILED [Wrong message ID]"});
  }
  peerBitField_ = message.getPayload();

  // Informs the PieceManager of the BitField received
  pieceManager_->addPeer(peerId_, peerBitField_);
  return {};
}

void PeerConnection::requestPiece() {
  Block* block = pieceManager_->nextRequest(peerId_);

  if (!block) return;

  int payload_length = 12;
  char temp[payload_length];
  // Needs to convert little-endian to big-endian
  uint32_t index = htonl(block->piece);
  uint32_t offset = htonl(block->offset);
  uint32_t length = htonl(block->length);
  std::memcpy(temp, &index, sizeof(int));
  std::memcpy(temp + 4, &offset, sizeof(int));
  std::memcpy(temp + 8, &length, sizeof(int));
  std::string payload;
  for (int i = 0; i < payload_length; i++) {
    payload += temp[i];
  }

  std::stringstream info;
  info << "Sending Request message to peer " << peer_->ip << " ";
  info << "[Piece: " << std::to_string(block->piece) << " ";
  info << "Offset: " << std::to_string(block->offset) << " ";
  info << "Length: " << std::to_string(block->length) << "]";
  std::string request_message = BitTorrentMessage(kRequest, payload).toString();
  sendData(sock_, request_message);
  requestPending_ = true;
}

tl::expected<void, PeerConnectionError> PeerConnection::sendInterested() {
  std::string interested_message = BitTorrentMessage(kInterested).toString();
  sendData(sock_, interested_message);
  return {};
}

tl::expected<void, PeerConnectionError> PeerConnection::receiveUnchoke() {
  BitTorrentMessage message = receiveMessage();
  if (message.getMessageId() != kUnchoke) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive Unchoke from peer: FAILED [Wrong message ID]"});
  }
  choked_ = false;
  return {};
}

tl::expected<void, PeerConnectionError>
PeerConnection::establishNewConnection() {
  if (auto handshake_result = performHandshake(); !handshake_result) {
    return tl::unexpected(handshake_result.error());
  }

  if (auto bitfield_result = receiveBitField(); !bitfield_result) {
    return tl::unexpected(bitfield_result.error());
  }

  sendInterested();

  return {};
}

std::string PeerConnection::createHandshakeMessage() {
  static constexpr char kProtocolName[] = "BitTorrent protocol";
  static constexpr size_t kReservedBytes = 8;

  std::ostringstream buffer;

  // Append protocol length and protocol name
  buffer.put(static_cast<char>(sizeof(kProtocolName) - 1));
  buffer.write(kProtocolName, sizeof(kProtocolName) - 1);

  // Append 8 reserved bytes (all set to zero)
  buffer.write("\0\0\0\0\0\0\0\0", kReservedBytes);

  // Append info hash and client ID
  buffer << utils::hexDecode(infoHash_) << clientId_;

  assert(buffer.str().size() == (sizeof(kProtocolName) - 1) + 49);
  return buffer.str();
}

BitTorrentMessage PeerConnection::receiveMessage() const {
  auto reply_option = receiveData(sock_, 0);
  std::string reply = reply_option.value();
  if (reply.empty()) {
    return BitTorrentMessage(kEepAlive);
  }

  auto message_id = static_cast<uint8_t>(reply[0]);
  std::string payload = reply.substr(1);
  return BitTorrentMessage(message_id, payload);
}

const std::string& PeerConnection::getPeerId() const { return peerId_; }

void PeerConnection::closeSock() {
  if (!sock_) {
    return;
  }

  close(sock_);
  sock_ = {};

  requestPending_ = false;

  if (!peerBitField_.empty()) {
    peerBitField_.clear();
    if (pieceManager_) {
      pieceManager_->removePeer(peerId_);
    }
  }
}
