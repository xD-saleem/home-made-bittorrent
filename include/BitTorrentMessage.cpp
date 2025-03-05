#include "BitTorrentMessage.h"

#include <netinet/in.h>

BitTorrentMessage::BitTorrentMessage(const uint8_t messageId,
                                     const std::string &payload)
    : messageLength(payload.length() + 1), id(messageId), payload(payload) {}

std::string BitTorrentMessage::toString() {
  // Convert messageLength to big-endian
  uint32_t big_endian_length = htonl(messageLength);
  const char *length_bytes = reinterpret_cast<const char *>(&big_endian_length);

  // Preallocate memory for efficiency
  std::string encoded_message;
  encoded_message.reserve(5 + payload.size());

  encoded_message.append(length_bytes, 4);
  encoded_message.push_back(static_cast<char>(id));
  encoded_message.append(payload);

  return encoded_message;
}

uint8_t BitTorrentMessage::getMessageId() const { return id; }

std::string BitTorrentMessage::getPayload() const { return payload; }
