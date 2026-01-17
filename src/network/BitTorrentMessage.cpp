#include "network/BitTorrentMessage.h"

#include <netinet/in.h>

BitTorrentMessage::BitTorrentMessage(const uint8_t messageId,
                                     const std::string& payload)
    : messageLength_(payload.length() + 1), id_(messageId), payload_(payload) {}

std::string BitTorrentMessage::toString() {
  // Convert the message length to network byte order (big-endian)
  uint32_t big_endian_length = htonl(messageLength_);

  // Create the encoded string, reserving enough space in one go
  // 4 bytes for length, 1 byte for id, and the payload
  std::string encoded;
  encoded.reserve(4 + 1 + payload_.size());

  // Append length bytes
  encoded.append(reinterpret_cast<char*>(&big_endian_length),
                 sizeof(big_endian_length));

  // Append the message id
  encoded.push_back(static_cast<char>(id_));

  // Append the payload
  encoded.append(payload_);

  return encoded;
}

uint8_t BitTorrentMessage::getMessageId() const { return id_; }

std::string BitTorrentMessage::getPayload() const { return payload_; }
