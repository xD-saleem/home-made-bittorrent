
#include <core/PeerRegistry.h>
#include <fmt/format.h>

#include <mutex>

#include "infra/Logger.h"

namespace {
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

  Logger::log(fmt::format("{}", current_count));
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

std::expected<void, PeerRegistryError> PeerRegistry::removePeer(
    const std::string& peerId) {
  // TODO(slim): Check in the outer function
  //  if (isComplete()) {
  //  return {};
  // }
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
