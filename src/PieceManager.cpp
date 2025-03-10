
#include "PieceManager.h"

#include <bencode/bencoding.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <tl/expected.hpp>

#include "BitTorrentMessage.h"
#include "Block.h"
#include "TorrentFileParser.h"
#include "utils.h"

#define BLOCK_SIZE 16384    // 2 ^ 14
#define MAX_PENDING_TIME 5  // 5 sec
#define PROGRESS_BAR_WIDTH 40
#define PROGRESS_DISPLAY_INTERVAL 1  // 1 sec

PieceManager::PieceManager(std::shared_ptr<TorrentFileParser> fileParser,
                           std::shared_ptr<Logger> logger,
                           const std::string &downloadPath,
                           const int maximumConnections)

    : pieceLength(fileParser->getPieceLength().value()),
      fileParser(fileParser),
      maximumConnections(maximumConnections) {
  missingPieces = initiatePieces();
  downloadedFile.open(downloadPath, std::ios::binary | std::ios::out);

  long fileSize = fileParser->getFileSize().value();
  downloadedFile.seekp(fileSize - 1);
  downloadedFile.write("", 1);

  startingTime = std::time(nullptr);
  std::thread progressThread([this] { this->trackProgress(); });
  progressThread.detach();
}

/**
 * Destructor of the PieceManager class. Frees all resources allocated.
 */
PieceManager::~PieceManager() {
  for (Piece *piece : missingPieces) delete piece;

  for (Piece *piece : ongoingPieces) delete piece;

  for (PendingRequest *pending : pendingRequests) delete pending;

  downloadedFile.close();
}

/**
 * Pre-constructs the list of pieces and blocks based on
 * the number of pieces and the size of the block.
 * @return a vector containing all the pieces in the file.
 */
std::vector<Piece *> PieceManager::initiatePieces() {
  tl::expected<std::vector<std::string>, TorrentFileParserError> pieceHashes =
      fileParser->splitPieceHashes();

  if (!pieceHashes.has_value()) {
    return std::vector<Piece *>();
  }
  auto pieceHashesValue = pieceHashes.value();
  totalPieces = pieceHashesValue.size();
  std::vector<Piece *> torrentPieces;
  missingPieces.reserve(totalPieces);

  tl::expected<long, TorrentFileParserError> totalLengthResult =
      fileParser->getFileSize();

  if (!totalLengthResult.has_value()) {
    logger->log("Failed to get file size");
    return std::vector<Piece *>();
  }

  long totalLength = totalLengthResult.value();

  // number of blocks in a normal piece (i.e. pieces that are not the last
  // one)
  int blockCount = ceil(static_cast<double>(pieceLength) / BLOCK_SIZE);
  long remLength = pieceLength;

  for (size_t i = 0; i < totalPieces; i++) {
    // The final piece is likely to have a smaller size.
    if (i == totalPieces - 1) {
      remLength = totalLength % pieceLength;
      blockCount =
          std::max((int)ceil(static_cast<double>(remLength) / BLOCK_SIZE), 1);
    }

    std::vector<Block *> blocks;
    blocks.reserve(blockCount);

    for (int offset = 0; offset < blockCount; offset++) {
      auto block = new Block;
      block->piece = i;
      block->status = missing;
      block->offset = offset * BLOCK_SIZE;

      int blockSize = BLOCK_SIZE;

      if (i == totalPieces - 1 && offset == blockCount - 1) {
        blockSize = remLength % BLOCK_SIZE;
      }

      block->length = blockSize;
      blocks.push_back(block);
    }

    auto piece = new Piece(i, blocks, pieceHashesValue[i]);
    torrentPieces.emplace_back(piece);
  }

  return torrentPieces;
}

bool PieceManager::isComplete() {
  lock.lock();
  size_t currentSize = havePieces.size();
  bool isComplete = (currentSize + 1 == totalPieces - 1);
  lock.unlock();
  return isComplete;
}

void PieceManager::addPeer(const std::string &peerId, std::string bitField) {
  lock.lock();
  peers[peerId] = bitField;
  lock.unlock();
  std::stringstream info;
  info << "Number of connections: " << std::to_string(peers.size())
       << "/" + std::to_string(maximumConnections);
}

/**
 * Updates the information about which pieces a peer has (i.e. reflects
 * a Have message).
 */
tl::expected<void, PieceManagerError> PieceManager::updatePeer(
    const std::string &peerId, int index) {
  lock.lock();
  if (peers.find(peerId) != peers.end()) {
    setPiece(peers[peerId], index);
    lock.unlock();
  } else {
    lock.unlock();
    return tl::unexpected(
        PieceManagerError{"Attempting to update a peer " + peerId +
                          " with whom a connection has not been established."});
  }
  return {};
}

/**
 * Removes a previously added peer in case of a lost connection.
 * @param peerId: Id of the peer to be removed.
 */
tl::expected<void, PieceManagerError> PieceManager::removePeer(
    const std::string &peerId) {
  if (isComplete()) {
    return {};
  }
  lock.lock();
  auto iter = peers.find(peerId);
  if (iter != peers.end()) {
    peers.erase(iter);
    lock.unlock();
    std::stringstream info;
    info << "Number of connections: " << std::to_string(peers.size())
         << "/" + std::to_string(maximumConnections);
  } else {
    lock.unlock();
    return tl::unexpected(
        PieceManagerError{"Attempting to remove a peer " + peerId +
                          " with whom a connection has not been established."});
  }
  return {};
}

std::vector<Piece *> PieceManager::getPieces() { return havePieces; }

/**
 * Retrieves the next block that should be requested from the given peer.
 * If there are no more blocks left to download or if this peer does not
 * have any of the missing pieces, None is returned
 * @return pointer to the Block struct to be requested.
 */
Block *PieceManager::nextRequest(std::string peerId) {
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

  lock.lock();
  if (missingPieces.empty()) {
    lock.unlock();
    return nullptr;
  }

  if (peers.find(peerId) == peers.end()) {
    lock.unlock();
    return nullptr;
  }

  Block *block = expiredRequest(peerId);
  if (!block) {
    block = nextOngoing(peerId);
    if (!block) block = getRarestPiece(peerId)->nextRequest();
  }
  lock.unlock();

  return block;
}

/**
 * Goes through previously requested blocks, if one has been in the
 * requested state for longer than `MAX_PENDING_TIME` returns the block to
 * be re-requested. If no pending blocks exist, None is returned
 */
Block *PieceManager::expiredRequest(std::string peerId) {
  time_t currentTime = std::time(nullptr);
  for (PendingRequest *pending : pendingRequests) {
    if (hasPiece(peers[peerId], pending->block->piece)) {
      // If the request has expired
      auto diff = std::difftime(currentTime, pending->timestamp);
      if (diff >= MAX_PENDING_TIME) {
        // Resets the timer for that request
        pending->timestamp = currentTime;
        return pending->block;
      }
    }
  }
  return nullptr;
}

/**
 * Iterates through the pieces that are currently being downloaded, and
 * returns the next Block to be requested or NULL if no Block is left to be
 * requested from the list of Pieces.
 */
Block *PieceManager::nextOngoing(std::string peerId) {
  for (Piece *piece : ongoingPieces) {
    if (hasPiece(peers[peerId], piece->index)) {
      Block *block = piece->nextRequest();
      if (block) {
        auto newPendingRequest = new PendingRequest;
        newPendingRequest->block = block;
        newPendingRequest->timestamp = std::time(nullptr);
        pendingRequests.push_back(newPendingRequest);
        return block;
      }
    }
  }
  return nullptr;
}

/**
 * Given the list of missing pieces, finds the rarest one (i.e. a piece
 * which is owned by the fewest number of peers).
 */
Piece *PieceManager::getRarestPiece(std::string peerId) {
  // Custom comparator to make sure that the map is ordered by the index of
  // the Piece.
  auto comp = [](const Piece *a, const Piece *b) {
    return a->index < b->index;
  };
  std::map<Piece *, int, decltype(comp)> pieceCount(comp);
  for (Piece *piece : missingPieces) {
    // If a connection has been established with the peer
    if (peers.find(peerId) != peers.end()) {
      if (hasPiece(peers[peerId], piece->index)) pieceCount[piece] += 1;
    }
  }

  Piece *rarest;
  int leastCount = INT16_MAX;
  for (auto const &[piece, count] : pieceCount) {
    if (count < leastCount) {
      leastCount = count;
      rarest = piece;
    }
  }

  missingPieces.erase(
      std::remove(missingPieces.begin(), missingPieces.end(), rarest),
      missingPieces.end());
  ongoingPieces.push_back(rarest);
  return rarest;
}

/**
 * This method is called when a block of data has been received successfully.
 * Once an entire Piece has been received, a SHA1 hash is computed on the data
 * of all the retrieved blocks in the Piece. The hash will be compared to
 * that from the Torrent meta-info. If a mismatch is detected, all the blocks
 * in the Piece will be reset to a missing state. If the hash matches, the
 * data in the Piece will be written to disk.
 */
tl::expected<void, PieceManagerError> PieceManager::blockReceived(
    std::string peerId, int pieceIndex, int blockOffset, std::string data) {
  // Removes the received block from pending requests
  PendingRequest *requestToRemove = nullptr;
  lock.lock();
  for (PendingRequest *pending : pendingRequests) {
    if (pending->block->piece == pieceIndex &&
        pending->block->offset == blockOffset) {
      requestToRemove = pending;
      break;
    }
  }

  pendingRequests.erase(std::remove(pendingRequests.begin(),
                                    pendingRequests.end(), requestToRemove),
                        pendingRequests.end());

  // Retrieves the Piece to which this Block belongs
  Piece *targetPiece = nullptr;
  for (Piece *piece : ongoingPieces) {
    if (piece->index == pieceIndex) {
      targetPiece = piece;
      break;
    }
  }
  lock.unlock();
  if (!targetPiece) {
    return tl::unexpected(PieceManagerError{
        "Received block for a piece that is not being downloaded."});
  }

  targetPiece->blockReceived(blockOffset, std::move(data));
  if (targetPiece->isComplete()) {
    // If the Piece is completed and the hash matches,
    // writes the Piece to disk
    if (targetPiece->isHashMatching()) {
      write(targetPiece);
      // Removes the Piece from the ongoing list
      lock.lock();
      ongoingPieces.erase(
          std::remove(ongoingPieces.begin(), ongoingPieces.end(), targetPiece),
          ongoingPieces.end());
      havePieces.push_back(targetPiece);
      piecesDownloadedInInterval++;
      lock.unlock();

      std::stringstream info;
      info << "(" << std::fixed << std::setprecision(2)
           << (((float)havePieces.size()) / (float)totalPieces * 100) << "%) ";
      info << std::to_string(havePieces.size()) + " / " +
                  std::to_string(totalPieces) + " Pieces downloaded...";
    } else {
      targetPiece->reset();
    }
  }
  return {};
}

/**
 * Writes the given Piece to disk.
 */
void PieceManager::write(Piece *piece) {
  long position = piece->index * fileParser->getPieceLength().value();
  downloadedFile.seekp(position);
  downloadedFile << piece->getData();
}

/**
 * Calculates the number of bytes downloaded.
 */
unsigned long PieceManager::bytesDownloaded() {
  lock.lock();
  unsigned long bytesDownloaded = havePieces.size() * pieceLength;
  lock.unlock();
  return bytesDownloaded;
}

/**
 * A function used by the progressThread to collect and calculate
 * statistics collected during the download and display them
 * in the form of a progress bar.
 */
void PieceManager::trackProgress() {
  usleep(pow(10, 6));
  while (!isComplete()) {
    displayProgressBar();
    // Resets the number of pieces downloaded to 0
    piecesDownloadedInInterval = 0;
    usleep(PROGRESS_DISPLAY_INTERVAL * pow(10, 6));
  }
}

/**
 * Creates and outputs a progress bar in stdout displaying
 * download statistics and progress.
 */
void PieceManager::displayProgressBar() {
  std::stringstream info;
  lock.lock();
  unsigned long downloadedPieces = havePieces.size();
  unsigned long downloadedLength = pieceLength * piecesDownloadedInInterval;

  // Calculates the average download speed in the last
  // PROGRESS_DISPLAY_INTERVAL in MB/s
  double avgDownloadSpeed =
      (double)downloadedLength / (double)PROGRESS_DISPLAY_INTERVAL;
  double avgDownloadSpeedInMBS = avgDownloadSpeed / pow(10, 6);

  info << "[Peers: " + std::to_string(peers.size()) + "/" +
              std::to_string(maximumConnections) + ", ";
  info << std::fixed << std::setprecision(2) << avgDownloadSpeedInMBS
       << " MB/s, ";

  // Estimates the remaining downloading time
  double timePerPiece =
      (double)PROGRESS_DISPLAY_INTERVAL / (double)piecesDownloadedInInterval;
  long remainingTime = ceil(timePerPiece * (totalPieces - downloadedPieces));
  info << "ETA: " << formatTime(remainingTime) << "]";

  double progress = (double)downloadedPieces / (double)totalPieces;
  int pos = PROGRESS_BAR_WIDTH * progress;
  info << "[";
  for (int i = 0; i < PROGRESS_BAR_WIDTH; i++) {
    if (i < pos)
      info << "=";
    else if (i == pos)
      info << ">";
    else
      info << " ";
  }
  info << "] ";
  info << std::to_string(downloadedPieces) + "/" + std::to_string(totalPieces) +
              " ";
  info << "[" << std::fixed << std::setprecision(2) << (progress * 100)
       << "%] ";

  time_t currentTime = std::time(nullptr);
  long timeSinceStart = floor(std::difftime(currentTime, startingTime));

  info << "in " << formatTime(timeSinceStart);
  std::cout << info.str() << "\r";
  std::cout.flush();
  lock.unlock();
  if (isComplete()) {
    std::cout << std::endl;
  }
}
