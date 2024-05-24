#include "Torrent_Client.h"

#include <cpr/cpr.h>
#include <fmt/core.h>

#include "Piece_Manager.h"
#include "Torrent_Client.h"
#include "Torrent_Parser.h"

// Constructor definition
Torrent_Client::Torrent_Client(const Torrent_Parser &parser,
                               const std::string &downloadPath)
    : parser(parser), downloadPath(downloadPath) {}

// Implement other member functions here...
int Torrent_Client::download() const {
  fmt::print("Downloading file to: {}  \n", downloadPath);
  Torrent_Parser parser = Torrent_Parser(downloadPath);
  std::string url = parser.getAnnounce();

  long fileSize = parser.getFileSize();
  long pieceLength = parser.getPieceLength();

  Piece_Manager pm = Piece_Manager(parser, downloadPath, 5);

  bool f = pm.isComplete();
  fmt::print("Is complete: {}", f);

  return 0;
}

int Torrent_Client::requestPeers(std::string &peerID, int port) const {
  // Implementation of requestPeers
  return 0;  // Example return
}

