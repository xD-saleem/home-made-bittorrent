#include "BitTorrentMessage.h"

#include <sstream>

BitTorrentMessage::BitTorrentMessage(const uint8_t id,
                                     const std::string& payload)
    : messageLength(payload.length() + 1), id(id), payload(payload) {}

std::string BitTorrentMessage::toString() {
  std::stringstream buffer;
  char* messageLengthAddr = (char*)&messageLength;
  std::string messageLengthStr;
  // Bytes are pushed in reverse order, assuming the data
  // is stored in little-endian order locally.
  for (int i = 0; i < 4; i++)
    messageLengthStr.push_back((char)messageLengthAddr[3 - i]);
  buffer << messageLengthStr;
  buffer << (char)id;
  buffer << payload;
  return buffer.str();
}

uint8_t BitTorrentMessage::getMessageId() const { return id; }

std::string BitTorrentMessage::getPayload() const { return payload; }
