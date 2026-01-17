
#include <core/PeerRegistry.h>
#include <fmt/format.h>

#include <mutex>

#include "infra/Logger.h"
#include "network/PeerRetriever.h"

// TODO(slim): add a injectable peerRegistry.
PeerRegistry::PeerRegistry() = default;

namespace {
bool hasPiece(const std::string& bitField, int index) {
  size_t byte_index = index / 8;
  int offset = index % 8;
  return (bitField[byte_index] >> (7 - offset) & 1) != 0;
}

void setPiece(std::string& bitField, int index) {
  int byte_index = index >> 3;
  int offset = index & 7;
  bitField[byte_index] |= (1 << (7 - offset));
}
}  // namespace

void PeerRegistry::addPeer(const std::string& peerId,
                           const std::string& bitField) {
  size_t current_count = 0;
  {
    std::lock_guard<std::mutex> lock(lock_);
    peers_[peerId] = std::move(bitField);
    current_count = peers_.size();
  }
}

std::expected<void, PeerRegistryError> PeerRegistry::updatePeer(
    const std::string& peerId, int index) {
  std::lock_guard<std::mutex> lock(lock_);

  auto it = peers_.find(peerId);

  if (it != peers_.end()) {
    setPiece(it->second, index);
    return {};
  }

  return std::unexpected(
      PeerRegistryError{"Attempting to update a peer " + peerId +
                        " with whom a connection has not been established."});
}

size_t PeerRegistry::peerCount() const { return peers_.size(); }

std::expected<std::string, PeerRegistryError> PeerRegistry::getPeer(
    const std::string& peerId) {
  std::lock_guard<std::mutex> lock(lock_);

  auto it = peers_.find(peerId);
  if (it != peers_.end()) {
    return it->second;
  }

  return std::unexpected(PeerRegistryError{"Attempting to get peer " + peerId});
}

bool PeerRegistry::peerHasPiece(const std::string& peerId,
                                int pieceIndex) const {
  auto it = peers_.find(peerId);
  if (it != peers_.end()) {
    std::string peer = it->second;
    return hasPiece(peer, pieceIndex);
  }
  // If the peer was not found, return false or handle the error case
  return false;
}

bool PeerRegistry::hasPeer(const std::string& peerId) const {
  auto it = peers_.find(peerId);

  if (it != peers_.end()) {
    return it->second != "";
  }

  return false;
}

std::expected<void, PeerRegistryError> PeerRegistry::removePeer(
    const std::string& peerId) {
  size_t remaining_peers = 0;
  bool peer_found = false;

  {
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = peers_.find(peerId);
    if (iter != peers_.end()) {
      peers_.erase(iter);
      remaining_peers = peers_.size();
      peer_found = true;
    }
  }

  if (peer_found) {
    Logger::log(fmt::format("Number of connections: {}", remaining_peers));
    return {};
  }

  // Return error if peer wasn't found
  return std::unexpected(PeerRegistryError{fmt::format(
      "Attempting to remove peer {} (connection not established).", peerId)});
}
