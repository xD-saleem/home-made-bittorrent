

#include "PiecePicker.h"

#include "core/Block.h"

Block* PiecePicker::nextRequest(const std::string& peerId) {
  // The algorithm tries to download the pieces in sequence and will try
  // to finish started pieces before starting with new pieces.
  //
  // 1. Check any pending blocks to see if any request should be reissued
  // due to timeout
  // 2. Check the ongoing pieces to get the next block to request
  // 3. Check if this peer have any of the missing pieces not yet started

  lock_.lock();
  if (missingPieces_.empty()) {
    lock_.unlock();
    return nullptr;
  }

  if (!peers_.contains(peerId)) {
    lock_.unlock();
    return nullptr;
  }

  Block* block = expiredRequest(peerId);
  if (!block) {
    block = nextOngoing(peerId);
    if (!block) block = getRarestPiece(peerId)->nextRequest();
  }
  lock_.unlock();

  return block;
}

Block* PiecePicker::expiredRequest(std::string peerId) {
  time_t current_time = std::time(nullptr);
  for (PendingRequest* pending : pendingRequests_) {
    if (utils::hasPiece(peers_[peerId], pending->block->piece)) {
      // If the request has expired
      auto diff = std::difftime(current_time, pending->timestamp);
      if (diff >= MAX_PENDING_TIME) {
        // Resets the timer for that request
        pending->timestamp = current_time;
        return pending->block;
      }
    }
  }
  return nullptr;
}
