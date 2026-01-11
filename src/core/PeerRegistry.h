
#ifndef BITTORRENTCLIENT_PEERREGISTRY_H
#define BITTORRENTCLIENT_PEERREGISTRY_H

#include <expected>
#include <mutex>
#include <string>
#include <unordered_map>

struct PeerRegistryError {
  std::string message;
};

class PeerRegistry {
 private:
  // Deps
  std::unordered_map<std::string, std::string> peers_;
  std::mutex lock_;

 public:
  explicit PeerRegistry();
  // Destructor
  ~PeerRegistry();

  void addPeer(const std::string& peerId, const std::string& bitField);

  std::expected<void, PeerRegistryError> updatePeer(const std::string& peerId,
                                                    int index);

  std::expected<void, PeerRegistryError> deletePeer(const std::string& peerId,
                                                    int index);

  std::expected<void, PeerRegistryError> removePeer(const std::string& peerId);
};

#endif  // BITTORRENTCLIENT_PEERREGISTRY_H
