
#include "Piece.h"

#include <openssl/sha.h>

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "utils.h"

Piece::Piece(int index, std::vector<std::unique_ptr<Block>> blocks,
             std::string hashValue)
    : index(index), hashValue_(std::move(hashValue)) {
  this->blocks = std::move(blocks);
}

void Piece::reset() {
  for (const auto& block : blocks) {
    block->status = kMissing;
  }
}

Block* Piece::nextRequest() {
  for (const auto& block : blocks) {
    if (block->status == kMissing) {
      block->status = kPending;
      return block.get();
    }
  }
  return nullptr;
}

tl::expected<void, PieceError> Piece::blockReceived(int offset,
                                                    std::string data) {
  for (const auto& block : blocks) {
    if (block->offset == offset) {
      block->status = kRetrieved;
      block->data = data;
      return {};
    }
  }
  return tl::make_unexpected(PieceError{"Block not found"});
}

/**
 * Checks if all Blocks within the Piece has been retrieved.
 */

bool Piece::isComplete() const {
  return std::all_of(blocks.begin(), blocks.end(),
                     [](const std::unique_ptr<Block>& block) {
                       return block->status == kRetrieved;
                     });
}

bool Piece::isHashMatching() {
  std::string data = getData();
  auto sha1ed_data = sha1(data);
  std::string piece_hash = hexDecode(sha1ed_data);
  return piece_hash == hashValue_;
}

std::string Piece::getData() {
  assert(isComplete());
  std::stringstream data;
  for (const auto& block : blocks) {
    data << block->data;
  }
  return std::string(data.str());
}
