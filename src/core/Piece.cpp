#include "core/Piece.h"

#include <openssl/sha.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "utils/utils.h"

Piece::Piece(int index, std::vector<std::unique_ptr<Block>> blocks,
             std::string hashValue)
    : index(index),
      blocks(std::move(blocks)),
      hash_value_(std::move(hashValue)) {}

void Piece::reset() {
  for (std::unique_ptr<Block>& block : blocks) {
    block->status = kMissing;
  }
}

Block* Piece::nextRequest() {
  for (auto& block : blocks) {
    if (block->status == kMissing) {
      block->status = kPending;
      return block.get();
    }
  }
  return nullptr;
}

tl::expected<void, PieceError> Piece::blockReceived(int offset,
                                                    std::string data) {
  for (std::unique_ptr<Block>& block : blocks) {
    if (block->offset == offset) {
      block->status = kRetrieved;
      block->data = data;
      return {};
    }
  }
  return tl::make_unexpected(PieceError{"Block not found"});
}

bool Piece::isComplete() const {
  return std::all_of(blocks.begin(), blocks.end(),
                     [](const std::unique_ptr<Block>& block) {
                       return block->status == kRetrieved;
                     });
}

bool Piece::isHashMatching() {
  std::string data = getData();
  auto sha1ed_data = utils::sha1(data);

  std::string piece_hash = utils::hexDecode(sha1ed_data);
  return piece_hash == hash_value_;
}

std::string Piece::getData() {
  assert(isComplete());
  std::stringstream data;
  for (std::unique_ptr<Block>& block : blocks) {
    data << block->data;
  }
  return std::string(data.str());
}
