#ifndef BITTORRENTCLIENT_BITTORRENTMESSAGE_H
#define BITTORRENTCLIENT_BITTORRENTMESSAGE_H

#include <cstdint>
#include <string>

enum MessageId {
  kEepAlive = -1,
  kChoke = 0,
  kUnchoke = 1,
  kInterested = 2,
  kNotInterested = 3,
  kHave = 4,
  kBitField = 5,
  kRequest = 6,
  kPiece = 7,
  kCancel = 8,
  kPort = 9,
  kSeeding = 10
};

class BitTorrentMessage {
 private:
  const uint32_t messageLength_;
  const uint8_t id_;
  const std::string payload_;

 public:
  explicit BitTorrentMessage(uint8_t id, const std::string &payload = "");
  std::string toString();
  [[nodiscard]] uint8_t getMessageId() const;
  [[nodiscard]] std::string getPayload() const;
};

#endif  // BITTORRENTCLIENT_BITTORRENTMESSAGE_H
