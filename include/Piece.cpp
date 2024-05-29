

#include "Piece.h"

#include <openssl/sha.h>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <loguru/loguru.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "utils.h"

Piece::Piece(int index, std::vector<Block*> blocks, std::string hashValue)
    : index(index), hashValue(std::move(hashValue)) {
  this->blocks = std::move(blocks);
}

/**
 * Destructor of the object. Releases all the allocated memory for blocks.
 */
Piece::~Piece() {
  for (Block* block : blocks) delete block;
}

/**
 * Resets the status of all Blocks in this Piece to Missing.
 */
void Piece::reset() {
  for (Block* block : blocks) block->status = missing;
}

/**
 * Finds and returns the next Block to be requested
 * (i.e the first Block that has the status Missing).
 * Changes that Block's status to Pending before returning.
 * If all Blocks are
 */
Block* Piece::nextRequest() {
  for (Block* block : blocks) {
    if (block->status == missing) {
      block->status = pending;
      return block;
    }
  }
  return nullptr;
}

std::string sha1(const std::string& str) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

  std::stringstream ss;
  for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<int>(hash[i]);
  }
  return ss.str();
}
/**
 * Updates the Block information by setting the status
 * of the Block specified by 'offset' to Retrieved.
 * @param offset: the offset of the Block within  the Piece.
 * @param data: the data contained in the Block.
 */
void Piece::blockReceived(int offset, std::string data) {
  for (Block* block : blocks) {
    if (block->offset == offset) {
      block->status = retrieved;
      block->data = data;
      return;
    }
  }
  throw std::runtime_error("Trying to complete a non-existing block " +
                           std::to_string(offset) + " in piece " +
                           std::to_string(index));
}

/**
 * Checks if all Blocks within the Piece has been retrieved.
 * Note that this function only checks if the data in the Blocks
 * has been received, it does not calculate the hash, and thus,
 * disregards the correctness of the data.
 */
bool Piece::isComplete() {
  return std::all_of(blocks.begin(), blocks.end(),
                     [](Block* block) { return block->status == retrieved; });
}

/**
 * Checks if the SHA1 hash for all the retrieved Block data matches
 * the Piece hash from the Torrent meta-info.
 */
bool Piece::isHashMatching() {
  std::string data = getData();
  auto sha1edData = sha1(data);

  std::string pieceHash = hexDecode(sha1edData);
  return pieceHash == hashValue;
}

/**
 * Concatenates the data in each Block, and returns it
 * as a whole. Note that for this to succeed, it must be
 * ensured that this Piece is complete.
 * @return the data contained in all the Blocks concatenated
 * as a string;
 */
std::string Piece::getData() {
  assert(isComplete());
  std::stringstream data;
  for (Block* block : blocks) data << block->data;
  return std::string(data.str());
}

