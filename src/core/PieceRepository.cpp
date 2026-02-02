
#include "core/PieceRepository.h"

#include <core/PeerRegistry.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <infra/Logger.h>

#include <algorithm>

#include "core/Piece.h"

#define BLOCK_SIZE 16384    // 2 ^ 14
#define MAX_PENDING_TIME 5  // 5 sec
#define PROGRESS_BAR_WIDTH 40
#define PROGRESS_DISPLAY_INTERVAL 1  // 1 sec

std::vector<std::unique_ptr<Piece>> PieceRepository::initiatePieces() {
  tl::expected<std::vector<std::string>, TorrentFileParserError> piece_hashes =
      fileParser_->splitPieceHashes();

  if (!piece_hashes.has_value()) {
    return {};
  }

  auto piece_hashes_value = piece_hashes.value();

  total_pieces_ = piece_hashes_value.size();
  missingPieces_.reserve(total_pieces_);

  std::vector<std::unique_ptr<Piece>> torrent_pieces;

  auto total_length_result = fileParser_->getFileSize();

  if (!total_length_result.has_value()) {
    Logger::log("Failed to get file size");
    return {};
  }

  int64_t total_length = total_length_result.value();

  auto piece_length = fileParser_->getPieceLength().value();

  // number of blocks in a normal piece
  int block_count = ceil(static_cast<double>(piece_length) / BLOCK_SIZE);

  int64_t rem_length = piece_length;

  for (size_t i = 0; i < total_pieces_; i++) {
    // The final piece is most likely to have a smaller size.
    if (i == total_pieces_ - 1) {
      rem_length = total_length % piece_length;
      block_count = std::max(
          static_cast<int>(ceil(static_cast<double>(rem_length) / BLOCK_SIZE)),
          1);
    }

    std::vector<std::unique_ptr<Block>> blocks;
    blocks.reserve(block_count);

    for (int offset = 0; offset < block_count; offset++) {
      std::unique_ptr<Block> block = std::make_unique<Block>();
      block->piece = i;
      block->status = kMissing;
      block->offset = offset * BLOCK_SIZE;

      int block_size = BLOCK_SIZE;

      if (i == total_pieces_ - 1 && offset == block_count - 1) {
        block_size = rem_length % BLOCK_SIZE;
      }

      block->length = block_size;
      blocks.push_back(std::move(block));
    }

    std::unique_ptr<Piece> piece =
        std::make_unique<Piece>(i, std::move(blocks), piece_hashes_value[i]);

    torrent_pieces.push_back(std::move(piece));
  }

  return torrent_pieces;
}

Piece* PieceRepository::acquireRarest(const std::string& peerId) {
  std::map<Piece*, int> piece_count;

  for (auto& piece_ptr : missingPieces_) {
    if (peerRegistry_->hasPeer(peerId)) {
      if (peerRegistry_->peerHasPiece(peerId, piece_ptr->index)) {
        piece_count[piece_ptr.get()] += 1;
      }
    }
  }

  // Find the piece with the least count
  Piece* rarest = nullptr;
  int least_count = INT_MAX;
  for (auto const& [piece, count] : piece_count) {
    if (count < least_count) {
      least_count = count;
      rarest = piece;
    }
  }

  if (!rarest) {
    // no piece available
    return nullptr;
  }
  //
  // // Move the rarest piece from missingPieces_ to ongoingPieces_
  auto it = std::ranges::find_if(
      missingPieces_,
      [rarest](const std::unique_ptr<Piece>& p) { return p.get() == rarest; });

  if (it != missingPieces_.end()) {
    ongoingPieces_.push_back(std::move(*it));
    missingPieces_.erase(it);
  }

  return rarest;
}
