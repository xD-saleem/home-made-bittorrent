
#include "Piece_Manager.h"

#include <Block.h>
#include <fmt/core.h>

#include <cmath>

#include "Torrent_Parser.h"

#define BLOCK_SIZE 16384

Piece_Manager::Piece_Manager(const Torrent_Parser& fileParser,
                             const std::string& downloadPath,
                             const int maximumConnections)
    : pieceLength(fileParser.getPieceLength()),
      fileParser(fileParser),
      maximumConnections(maximumConnections) {
  downloadedFile.open("./saleem.torrent", std::ios::binary | std::ios::out);
  downloadedFile.seekp(fileParser.getFileSize() - 1);
  downloadedFile.write("", 1);
}

Piece_Manager::~Piece_Manager() {
  for (Piece* piece : missingPieces) delete piece;
  for (Piece* piece : ongoingPieces) delete piece;
  for (PendingRequest* pending : pendingRequests) delete pending;
  downloadedFile.close();
}

bool Piece_Manager::isComplete() {
  lock.lock();
  bool isComplete = havePieces.size() == totalPieces;
  lock.unlock();
  return isComplete;
}

void Piece_Manager::addPeer(const std::string& peerId, std::string bitField) {
  lock.lock();
  peers[peerId] = bitField;
  lock.unlock();
  fmt::print("Peer {} added\n", peerId);
}

void Piece_Manager::removePeer(const std::string& peerId) {
  // If download is not complete, do not remove peer
  if (!isComplete()) return;

  lock.lock();
  peers.erase(peerId);
  lock.unlock();
  fmt::print("Peer {} removed\n", peerId);
}

void Piece_Manager::updatePeer(const std::string& peerId, int index) {
  lock.lock();
  peers[peerId];
  lock.unlock();
}

std::vector<Piece*> Piece_Manager::initiatePieces() {
  std::optional<std::vector<std::string>> pieceHashes =
      fileParser.splitPieceHashes();

  if (!pieceHashes.has_value()) {
    totalPieces = pieceHashes->size();
    auto pieceHashesValue = pieceHashes.value();

    std::vector<Piece*> torrentPieces;
    missingPieces.reserve(totalPieces);

    long totalLength = fileParser.getFileSize();

    int blockCount = ceil(pieceLength / BLOCK_SIZE);
    long remLength = pieceLength;

    for (int i = 0; i < totalPieces; i++) {
      if (i == totalPieces - 1) {
        remLength = totalLength % pieceLength;
        blockCount = std::max((int)ceil(remLength / BLOCK_SIZE), 1);
      }

      std::vector<Block*> blocks;
      blocks.reserve(blockCount);

      for (int offset = 0; offset < blockCount; offset++) {
        Block* block = new Block();
        block->piece = i;
        block->status = missing;
        block->offset = offset * BLOCK_SIZE;
        int blockSize = BLOCK_SIZE;

        if (i == totalPieces - 1 && offset == blockCount - 1) {
          blockSize = remLength % BLOCK_SIZE;
          block->length = blockSize;
          blocks.push_back(block);
        }
      }
      std::string d = pieceHashesValue[i];

      Piece* piece = new Piece(i, blocks, d);
      torrentPieces.emplace_back(piece);
    }
    return torrentPieces;
  }
  return std::vector<Piece*>();
}
