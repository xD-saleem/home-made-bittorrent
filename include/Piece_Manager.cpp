
#include "Piece_Manager.h"

#include <fmt/core.h>

#include "Torrent_Parser.h"

Piece_Manager::Piece_Manager(const Torrent_Parser& fileParser,
                             const std::string& downloadPath,
                             const int maximumConnections)
    : pieceLength(fileParser.getPieceLength()),
      fileParser(fileParser),
      maximumConnections(maximumConnections) {
  downloadedFile.open(downloadPath, std::ios::binary | std::ios::out);
  downloadedFile.seekp(fileParser.getFileSize() - 1);
  downloadedFile.write("", 1);
}

Piece_Manager::~Piece_Manager() {
  // TODO - Fix this
  //  for (Piece* piece : missingPieces) delete piece;
  //
  //  for (Piece* piece : ongoingPieces) delete piece;
  //
  //  for (PendingRequest* pending : pendingRequests) delete pending;

  downloadedFile.close();
}

