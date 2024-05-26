
#include "Piece.h"

#include <string>
#include <utility>
#include <vector>

#include "Piece.h"

Piece::Piece(int index, std::vector<Block*> blocks, std::string hashValue)
    : index(index), hashValue(std::move(hashValue)) {
  this->blocks = std::move(blocks);
}

Piece::~Piece() {
  for (Block* block : blocks) delete block;
}

void Piece::reset() {
  for (Block* block : blocks) block->status = missing;
}

