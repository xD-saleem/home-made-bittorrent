#include "PeerConnection.h"

#include <fmt/base.h>
#include <fmt/core.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
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
    : queue(queue), clientId(std::move(clientId)),
      infoHash(std::move(infoHash)), pieceManager(pieceManager) {}

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

/**
 * Terminates the peer connection
 */
void PeerConnection::stop() { terminated = true; }

std::string PeerConnection::XXX() {
  const std::string protocol = "BitTorrent protocol";
  std::stringstream buffer;
  buffer << (char)protocol.length();
  buffer << protocol;
  std::string reserved;
  for (int i = 0; i < 8; i++)
    reserved.push_back('\0');
  buffer << reserved;
  buffer << hexDecode(infoHash);
  buffer << clientId;
  assert(buffer.str().length() == protocol.length() + 49);
  return buffer.str();
}

/**
 * Establishes a TCP connection with the peer and sent it our initial
 * BitTorrent handshake message. Waits for its reply, and compares the info
 * hash contained in its response message with the info hash we calculated
 * from the Torrent file. If they do not match, close the connection.
 */
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
  fmt::println("dd #### {}", reply);

  if (reply.empty()) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive handshake from peer: FAILED [No response from peer]"});
  }
  peerId = reply.substr(PEER_ID_STARTING_POS, HASH_LEN);

  // Compare the info hash from the peer's reply message with the info hash we
  // sent. If the two values are not the same, close the connection and raise
  // an exception.
  std::string receivedInfoHash = reply.substr(INFO_HASH_STARTING_POS, HASH_LEN);
  if ((receivedInfoHash == infoHash) != 0) {
    return tl::make_unexpected(
        PeerConnectionError{"Perform handshake with peer " + peer->ip +
                            ": FAILED [Received mismatching info hash]"});
  }

  return {};
}

/**
 * Receives and reads the message which contains BitField from the peer.
 */
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

tl::expected<void, PeerConnectionError> PeerConnection::sendBitField() {
  try {
    // 1. Calculate Bitfield Size and Data
    auto pieces = pieceManager->getPieces();
    int numPieces = pieces.size();

    int bitfieldSize = (numPieces + 7) / 8; // Calculate bytes needed (round up)
    std::vector<char> bitfield(bitfieldSize, 0); // Initialize to 0

    for (int i = 0; i < numPieces; ++i) {
      bitfield[i / 8] |= (1 << (7 - (i % 8))); // Set the corresponding bit
    }

    // 2. Construct the Message

    int messageLength = 1 + bitfieldSize; // 1 for message ID + bitfield size
    std::vector<char> message(4 + messageLength); // 4 for length prefix

    // Length Prefix (Big-Endian)
    message[0] = (messageLength >> 24) & 0xFF;
    message[1] = (messageLength >> 16) & 0xFF;
    message[2] = (messageLength >> 8) & 0xFF;
    message[3] = messageLength & 0xFF;

    // Message ID
    message[4] = 5; // Bitfield message ID

    // Bitfield Data
    for (int i = 0; i < bitfieldSize; ++i) {
      message[5 + i] = bitfield[i];
    }

    // 3. Send the Message

    int bytesSent = send(0, message.data(), message.size(),
                         0); // socketFD is your socket

    if (bytesSent < 0) {
      // LOG_F(ERROR, "Error sending bitfield: %s", strerror(errno));
      // return false; // Return false on error
    }
    if (bytesSent != message.size()) {
      // LOG_F(ERROR, "Error sending bitfield: sent partial message (%d of %d)",
      // bytesSent, message.size());
      // return false; // Return false on error
      return {};
    }

    return {};

  } catch (const std::exception &e) {
    return {};
  }
}
/**
 * Sends a request message to the peer for the next block
 * to be downloaded.
 */
void PeerConnection::requestPiece() {
  Block *block = pieceManager->nextRequest(peerId);

  if (!block)
    return;

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
  for (int i = 0; i < payloadLength; i++)
    payload += (char)temp[i];

  std::stringstream info;
  info << "Sending Request message to peer " << peer->ip << " ";
  info << "[Piece: " << std::to_string(block->piece) << " ";
  info << "Offset: " << std::to_string(block->offset) << " ";
  info << "Length: " << std::to_string(block->length) << "]";
  std::string requestMessage = BitTorrentMessage(request, payload).toString();
  sendData(sock, requestMessage);
  requestPending = true;
}

/**
 * Send an Interested message to the peer.
 */
void PeerConnection::sendInterested() {
  std::string interestedMessage = BitTorrentMessage(interested).toString();
  sendData(sock, interestedMessage);
}

void PeerConnection::sendSeed() {
  std::string seedMessge = BitTorrentMessage(have).toString();
  sendData(sock, seedMessge);
}

/**
 * Receives and reads the Unchoke message from the peer.
 * If the received message does not match the expected Unchoke, raise an
 * error.
 */
tl::expected<void, PeerConnectionError> PeerConnection::receiveUnchoke() {
  BitTorrentMessage message = receiveMessage();
  if (message.getMessageId() != unchoke) {
    return tl::make_unexpected(PeerConnectionError{
        "Receive Unchoke from peer: FAILED [Wrong message ID]"});
  }
  choked = false;
  return {};
}

/**
 * This function establishes a TCP connection with the peer and performs
 * the following actions:
 *
 * 1. Sends the peer a BitTorrent handshake message, waits for its reply and
 * compares the info hashes.
 * 2. Receives and stores the BitField from the peer.
 * 3. Send an Interested message to the peer.
 *
 * Returns true if a stable connection has been successfully established,
 * false otherwise.
 *
 * To understand the details, the following links can be helpful:
 * - https://blog.jse.li/posts/torrent/
 * - https://markuseliasson.se/article/bittorrent-in-python/
 * - https://wiki.theory.org/BitTorrentSpecification#Handshake
 */
bool PeerConnection::establishNewConnection() {
  try {
    // TODO return error instead of throwing
    performHandshake();
    receiveBitField();
    sendInterested();
    return true;
  } catch (const std::runtime_error &e) {
    return false;
  }
}

bool PeerConnection::establishSeedNewConnection() {
  try {
    performHandshake();
    sendBitField();
    sendSeed();
    return true;
  } catch (const std::runtime_error &e) {
    return false;
  }
}

/**
 * Generates the initial handshake message to send to the peer.
 * Essentially, the handshake message has the following structure:
 *
 * handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
 * pstrlen: string length of <pstr>, as a single raw byte
 * pstr: string identifier of the protocol
 * reserved: eight (8) reserved bytes.
 * info_hash: 20-byte SHA1 hash Torrent file.
 * peer_id: 20-byte string used as a unique ID for the client.
 *
 * The detailed description can be found at:
 * https://wiki.theory.org/BitTorrentSpecification#Handshake
 *
 * @return a string representation of the Torrent handshake message.
 */
std::string PeerConnection::createHandshakeMessage() {
  const std::string protocol = "BitTorrent protocol";
  std::stringstream buffer;
  buffer << (char)protocol.length();
  buffer << protocol;
  std::string reserved;
  for (int i = 0; i < 8; i++)
    reserved.push_back('\0');
  buffer << reserved;
  buffer << hexDecode(infoHash);
  buffer << clientId;
  assert(buffer.str().length() == protocol.length() + 49);
  return buffer.str();
}

/**
 * A wrapper around the receiveData() function, in a sense that it returns
 * a BitTorrentMessage object so that parameters such as message length, id
 * and payload can be accessed more easily.
 */
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

/**
 * Retrieves the peer ID of the peer that is currently in contact with us.
 */
const std::string &PeerConnection::getPeerId() const { return peerId; }

/**
 * Closes the socket to a peer and sets the
 * instance variable 'sock' to null;
 */
void PeerConnection::closeSock() {
  if (sock) {
    // Close socket
    close(sock);
    sock = {};
    requestPending = false;
    // If the peer has been added to piece manager, remove it
    if (!peerBitField.empty()) {
      peerBitField.clear();
      pieceManager->removePeer(peerId);
    }
  }
}

tl::expected<void, PeerConnectionError> PeerConnection::seed() {
  while (true) {
    peer = queue->pop_front();
    if (peer->ip == DUMMY_PEER_IP) {
      return {};
    }

    try {
      if (establishSeedNewConnection()) {
        while (true) { // Keep connection alive for seeding
          BitTorrentMessage message = receiveMessage();

          if (message.getMessageId() > 10) {
            return tl::make_unexpected(PeerConnectionError{
                "Received invalid message Id from peer " + peerId});
          }

          switch (message.getMessageId()) {
          case choke:
            choked = true;
            break;

          case unchoke:
            choked = false;
            break;

          case have: {
            std::string payload = message.getPayload();
            int pieceIndex = bytesToInt(payload);
            pieceManager->updatePeer(peerId, pieceIndex);
            break;
          }

          case request: {  // Handle piece requests!
            if (!choked) { // Only send if not choked
              std::string payload = message.getPayload();
              int index = bytesToInt(payload.substr(0, 4));
              int begin = bytesToInt(payload.substr(4, 4));
              int length =
                  bytesToInt(payload.substr(8, 4)); // Get requested length

              // if (index >= 0 && index < pieces.size() && // Validate index
              //     begin >= 0 && begin < pieceSize && // Validate begin
              //     offset begin + length <= pieceSize &&     // Validate
              //     length pieces[index]) { // Check if you HAVE the piece
              //
              //   std::string blockData = pieceManager->getPieceBlock(
              //       index, begin,
              //       length); // Get the block from pieceManager
              //
              //   if (!blockData.empty()) {
              //     BitTorrentMessage pieceMessage(piece, index, begin,
              //                                    blockData);
              //     sendMessage(pieceMessage);
              //     LOG_F(INFO, "Sent piece %d, begin %d, length %d to peer
              //     %s",
              //           index, begin, length, peer->ip);
              //   } else {
              //     LOG_F(WARNING,
              //           "Failed to get piece block %d, begin %d, length
              //           %d", index, begin, length);
              //   }
              // } else {
              //   LOG_F(WARNING,
              //         "Invalid piece request: index=%d, begin=%d,
              //         length=%d, " "choked=%d, hasPiece=%d", index, begin,
              //         length, choked, (pieces[index] ? 1 : 0));
              // }
            }
            break;
          }

          case cancel: { // Handle cancel requests (optional but good
            break;
          }

          default:
            break;
          }
        } // End inner while loop (connection handling)
      }
    } catch (std::exception &e) {
      closeSock();
    }
  }
  return {};
}
