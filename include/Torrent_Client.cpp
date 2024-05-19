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
  // Implementation of download
  return 0;  // Example return
}

int Torrent_Client::requestPeers(std::string &peerID, int port) const {
  // Implementation of requestPeers
  return 0;  // Example return
}
