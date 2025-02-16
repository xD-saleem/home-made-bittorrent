#include "BitTorrentMessage.h"

#include <netinet/in.h>

BitTorrentMessage::BitTorrentMessage(const uint8_t id,
                                     const std::string &payload)
    : messageLength(payload.length() + 1), id(id), payload(payload) {}

std::string BitTorrentMessage::toString() {
  // Convert messageLength to big-endian
  uint32_t bigEndianLength = htonl(messageLength);
  const char *lengthBytes = reinterpret_cast<const char *>(&bigEndianLength);

  // Preallocate memory for efficiency
  std::string encodedMessage;
  encodedMessage.reserve(5 + payload.size());

  encodedMessage.append(lengthBytes, 4);
  encodedMessage.push_back(static_cast<char>(id));
  encodedMessage.append(payload);

  return encodedMessage;
}

uint8_t BitTorrentMessage::getMessageId() const { return id; }

std::string BitTorrentMessage::getPayload() const { return payload; }
