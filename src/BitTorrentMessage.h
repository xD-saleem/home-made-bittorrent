#ifndef BITTORRENTCLIENT_BITTORRENTMESSAGE_H
#define BITTORRENTCLIENT_BITTORRENTMESSAGE_H

#include <cstdint>
#include <string>

enum MessageId {
  keepAlive = -1,
  choke = 0,
  unchoke = 1,
  interested = 2,
  notInterested = 3,
  have = 4,
  bitField = 5,
  request = 6,
  piece = 7,
  cancel = 8,
  port = 9,
  seeding = 10
};

class BitTorrentMessage {
 private:
  const uint32_t messageLength;
  const uint8_t id;
  const std::string payload;

 public:
  explicit BitTorrentMessage(uint8_t id, const std::string &payload = "");
  std::string toString();
  [[nodiscard]] uint8_t getMessageId() const;
  [[nodiscard]] std::string getPayload() const;
};

#endif  // BITTORRENTCLIENT_BITTORRENTMESSAGE_H
