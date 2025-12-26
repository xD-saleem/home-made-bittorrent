#include "BitTorrentMessage.h"

#include <netinet/in.h>

#include <array>
#include <cstring>

BitTorrentMessage::BitTorrentMessage(const uint8_t messageId,
                                     const std::string& payload)
    : messageLength_(payload.length() + 1), id_(messageId), payload_(payload) {}

std::string BitTorrentMessage::toString() {
  uint32_t big_endian_length = htonl(messageLength_);

  std::array<char, 4> length_bytes{};
  std::memcpy(length_bytes.data(), &big_endian_length,
              sizeof(big_endian_length));

  std::string encoded;
  encoded.reserve(length_bytes.size() + 1 + payload_.size());

  encoded.append(length_bytes.data(), length_bytes.size());
  encoded.push_back(static_cast<char>(id_));
  encoded.append(payload_);

  return encoded;
}

uint8_t BitTorrentMessage::getMessageId() const { return id_; }

std::string BitTorrentMessage::getPayload() const { return payload_; }
