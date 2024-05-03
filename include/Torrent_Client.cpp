
#include "Torrent_Client.h"

#include <cpr/cpr.h>
#include <fmt/core.h>

int Torrent_Client::download() const { return 0; }

int Torrent_Client::requestPeers(std::string &peerID, int port) const {
  std::string announceUrl = parser.getAnnounce();
  const std::string infoHash = parser.getInfoHash();

  std::string url =
      parser.buildTrackerUrl(announceUrl, infoHash, "peer_id", 6881, 0, 0, 0);

  fmt::print("Requesting peers from {}\n", url);
  // cpr::Response r = cpr::Get(
  //     cpr::Url{"https://api.github.com/repos/whoshuu/cpr/contributors"});
  //
  // fmt::print("########### {}", r.status_code);  // 200

  return 0;
}

