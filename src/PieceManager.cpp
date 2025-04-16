
#include "PieceManager.h"

#include <bencode/bencoding.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <tl/expected.hpp>
#include <utility>

#include "Block.h"
#include "TorrentFileParser.h"
#include "utils.h"

#define BLOCK_SIZE 16384    // 2 ^ 14
#define MAX_PENDING_TIME 5  // 5 sec
#define PROGRESS_BAR_WIDTH 40
#define PROGRESS_DISPLAY_INTERVAL 1  // 1 sec

PieceManager::PieceManager(const std::shared_ptr<TorrentFileParser> &fileParser,
                           const std::string &downloadPath,
                           const int maximumConnections)
    : pieceLength_(fileParser->getPieceLength().value()),
      fileParser_(fileParser),
      maximumConnections_(maximumConnections) {
  missingPieces_ = initiatePieces();
  downloadedFile_.open(downloadPath, std::ios::binary | std::ios::out);

  int64_t file_size = fileParser->getFileSize().value();
  downloadedFile_.seekp(file_size - 1);
  downloadedFile_.write("", 1);

  startingTime_ = std::time(nullptr);
  std::thread progress_thread([this] { this->trackProgress(); });
  progress_thread.detach();
}

/**
 * Destructor of the PieceManager class. Frees all resources allocated.
 */
PieceManager::~PieceManager() {
  for (Piece *piece : missingPieces_) delete piece;

  for (Piece *piece : ongoingPieces_) delete piece;

  for (PendingRequest *pending : pendingRequests_) delete pending;

  downloadedFile_.close();
}

/**
 * Pre-constructs the list of pieces and blocks based on
 * the number of pieces and the size of the block.
 * @return a vector containing all the pieces in the file.
 */
std::vector<Piece *> PieceManager::initiatePieces() {
  tl::expected<std::vector<std::string>, TorrentFileParserError> piece_hashes =
      fileParser_->splitPieceHashes();

  if (!piece_hashes.has_value()) {
    return {};
  }
  auto piece_hashes_value = piece_hashes.value();
  total_pieces = piece_hashes_value.size();
  std::vector<Piece *> torrent_pieces;
  missingPieces_.reserve(total_pieces);

  tl::expected<int64_t, TorrentFileParserError> total_length_result =
      fileParser_->getFileSize();

  if (!total_length_result.has_value()) {
    Logger::log("Failed to get file size");
    return {};
  }

  int64_t total_length = total_length_result.value();

  // number of blocks in a normal piece (i.e. pieces that are not the last
  // one)
  int block_count = ceil(static_cast<double>(pieceLength_) / BLOCK_SIZE);
  int64_t rem_length = pieceLength_;

  for (size_t i = 0; i < total_pieces; i++) {
    // The final piece is likely to have a smaller size.
    if (i == total_pieces - 1) {
      rem_length = total_length % pieceLength_;
      block_count = std::max(
          static_cast<int>(ceil(static_cast<double>(rem_length) / BLOCK_SIZE)),
          1);
    }

    std::vector<Block *> blocks;
    blocks.reserve(block_count);

    for (int offset = 0; offset < block_count; offset++) {
      auto *block = new Block;
      block->piece = i;
      block->status = kMissing;
      block->offset = offset * BLOCK_SIZE;

      int block_size = BLOCK_SIZE;

      if (i == total_pieces - 1 && offset == block_count - 1) {
        block_size = rem_length % BLOCK_SIZE;
      }

      block->length = block_size;
      blocks.push_back(block);
    }

    auto *piece = new Piece(i, blocks, piece_hashes_value[i]);
    torrent_pieces.emplace_back(piece);
  }

  return torrent_pieces;
}

bool PieceManager::isComplete() {
  lock_.lock();
  size_t current_size = havePieces.size();
  const size_t header = 4;
  bool is_complete = (current_size + header == total_pieces);
  lock_.unlock();
  return is_complete;
}

void PieceManager::addPeer(const std::string &peerId, std::string bitField) {
  lock_.lock();
  peers_[peerId] = std::move(bitField);
  lock_.unlock();
  std::stringstream info;
  info << "Number of connections: " << std::to_string(peers_.size())
       << "/" + std::to_string(maximumConnections_);
}

/**
 * Updates the information about which pieces a peer has (i.e. reflects
 * a Have message).
 */
tl::expected<void, PieceManagerError> PieceManager::updatePeer(
    const std::string &peerId, int index) {
  lock_.lock();
  if (peers_.find(peerId) != peers_.end()) {
    setPiece(peers_[peerId], index);
    lock_.unlock();
  } else {
    lock_.unlock();
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
  lock_.lock();
  auto iter = peers_.find(peerId);
  if (iter != peers_.end()) {
    peers_.erase(iter);
    lock_.unlock();
    std::stringstream info;
    info << "Number of connections: " << std::to_string(peers_.size())
         << "/" + std::to_string(maximumConnections_);
  } else {
    lock_.unlock();
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
Block *PieceManager::nextRequest(const std::string peerId) {
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

  if (peers_.find(peerId) == peers_.end()) {
    lock_.unlock();
    return nullptr;
  }

  Block *block = expiredRequest(peerId);
  if (!block) {
    block = nextOngoing(peerId);
    if (!block) block = getRarestPiece(peerId)->nextRequest();
  }
  lock_.unlock();

  return block;
}

/**
 * Goes through previously requested blocks, if one has been in the
 * requested state for longer than `MAX_PENDING_TIME` returns the block to
 * be re-requested. If no pending blocks exist, None is returned
 */
Block *PieceManager::expiredRequest(std::string peerId) {
  time_t current_time = std::time(nullptr);
  for (PendingRequest *pending : pendingRequests_) {
    if (hasPiece(peers_[peerId], pending->block->piece)) {
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

/**
 * Iterates through the pieces that are currently being downloaded, and
 * returns the next Block to be requested or NULL if no Block is left to be
 * requested from the list of Pieces.
 */
Block *PieceManager::nextOngoing(std::string peerId) {
  for (Piece *piece : ongoingPieces_) {
    if (hasPiece(peers_[peerId], piece->index)) {
      Block *block = piece->nextRequest();
      if (block) {
        auto *new_pending_request = new PendingRequest;
        new_pending_request->block = block;
        new_pending_request->timestamp = std::time(nullptr);
        pendingRequests_.push_back(new_pending_request);
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
  std::map<Piece *, int, decltype(comp)> piece_count(comp);
  for (Piece *piece : missingPieces_) {
    // If a connection has been established with the peer
    if (peers_.find(peerId) != peers_.end()) {
      if (hasPiece(peers_[peerId], piece->index)) piece_count[piece] += 1;
    }
  }

  Piece *rarest;
  int least_count = INT16_MAX;
  for (auto const &[piece, count] : piece_count) {
    if (count < least_count) {
      least_count = count;
      rarest = piece;
    }
  }

  missingPieces_.erase(
      std::remove(missingPieces_.begin(), missingPieces_.end(), rarest),
      missingPieces_.end());
  ongoingPieces_.push_back(rarest);
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
    int pieceIndex, int blockOffset, std::string data) {
  // Removes the received block from pending requests
  PendingRequest *request_to_remove = nullptr;
  lock_.lock();
  for (PendingRequest *pending : pendingRequests_) {
    if (pending->block->piece == pieceIndex &&
        pending->block->offset == blockOffset) {
      request_to_remove = pending;
      break;
    }
  }

  pendingRequests_.erase(std::remove(pendingRequests_.begin(),
                                     pendingRequests_.end(), request_to_remove),
                         pendingRequests_.end());

  // Retrieves the Piece to which this Block belongs
  Piece *target_piece = nullptr;
  for (Piece *piece : ongoingPieces_) {
    if (piece->index == pieceIndex) {
      target_piece = piece;
      break;
    }
  }
  lock_.unlock();
  if (!target_piece) {
    return tl::unexpected(PieceManagerError{
        "Received block for a piece that is not being downloaded."});
  }

  target_piece->blockReceived(blockOffset, std::move(data));
  if (target_piece->isComplete()) {
    // If the Piece is completed and the hash matches,
    // writes the Piece to disk
    if (target_piece->isHashMatching()) {
      write(target_piece);
      // Removes the Piece from the ongoing list
      lock_.lock();
      ongoingPieces_.erase(std::remove(ongoingPieces_.begin(),
                                       ongoingPieces_.end(), target_piece),
                           ongoingPieces_.end());
      havePieces.push_back(target_piece);
      piecesDownloadedInInterval_++;
      lock_.unlock();

      std::stringstream info;
      info << "(" << std::fixed << std::setprecision(2)
           << ((static_cast<float>(havePieces.size())) /
               static_cast<float>(total_pieces) * 100)
           << "%) ";
      info << std::to_string(havePieces.size()) + " / " +
                  std::to_string(total_pieces) + " Pieces downloaded...";
    } else {
      target_piece->reset();
    }
  }
  return {};
}

/**
 * Writes the given Piece to disk.
 */
void PieceManager::write(Piece *piece) {
  int64_t position = piece->index * fileParser_->getPieceLength().value();
  downloadedFile_.seekp(position);
  downloadedFile_ << piece->getData();
}

/**
 * Calculates the number of bytes downloaded.
 */
uint64_t PieceManager::bytesDownloaded() {
  lock_.lock();
  uint64_t bytes_downloaded = havePieces.size() * pieceLength_;
  lock_.unlock();
  return bytes_downloaded;
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
    piecesDownloadedInInterval_ = 0;
    usleep(PROGRESS_DISPLAY_INTERVAL * pow(10, 6));
  }
}

/**
 * Creates and outputs a progress bar in stdout displaying
 * download statistics and progress.
 */
void PieceManager::displayProgressBar() {
  std::stringstream info;
  lock_.lock();
  uint64_t downloaded_pieces = havePieces.size();
  uint64_t downloaded_length = pieceLength_ * piecesDownloadedInInterval_;

  // Calculates the average download speed in the last
  // PROGRESS_DISPLAY_INTERVAL in MB/s
  double avg_download_speed =
      static_cast<double>(downloaded_length) / PROGRESS_DISPLAY_INTERVAL;
  double avg_download_speed_in_mbs = avg_download_speed / pow(10, 6);

  info << "[Peers: " + std::to_string(peers_.size()) + "/" +
              std::to_string(maximumConnections_) + ", ";
  info << std::fixed << std::setprecision(2) << avg_download_speed_in_mbs
       << " MB/s, ";

  // Estimates the remaining downloading time
  double time_per_piece = PROGRESS_DISPLAY_INTERVAL /
                          static_cast<double>(piecesDownloadedInInterval_);
  int64_t remaining_time =
      ceil(time_per_piece * (total_pieces - downloaded_pieces));
  info << "ETA: " << formatTime(remaining_time) << "]";

  double progress = static_cast<double>(downloaded_pieces) /
                    static_cast<double>(total_pieces);
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
  info << std::to_string(downloaded_pieces) + "/" +
              std::to_string(total_pieces) + " ";
  info << "[" << std::fixed << std::setprecision(2) << (progress * 100)
       << "%] ";

  time_t current_time = std::time(nullptr);
  int64_t time_since_start = floor(std::difftime(current_time, startingTime_));

  info << "in " << formatTime(time_since_start);
  std::cout << info.str() << "\r";
  std::cout.flush();
  lock_.unlock();
  if (isComplete()) {
    std::cout << std::endl;
  }
}
