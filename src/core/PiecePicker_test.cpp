
#ifndef BITTORRENTCLIENT_PIECEPICKER_H
#define BITTORRENTCLIENT_PIECEPICKER_H

#include <mutex>
#include <string>

#include "core/PeerRegistry.h"
#include "core/Piece.h"

// The algorithm implemented for which piece to retrieve is a simple
// one. This should preferably be replaced with an implementation of
// "rarest-piece-first" algorithm instead.
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
