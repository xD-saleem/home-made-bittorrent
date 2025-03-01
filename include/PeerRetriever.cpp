#include "PeerRetriever.h"

#include <bencode/bencoding.h>
#include <cpr/cpr.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <iostream>
#include <string>
#include <tl/expected.hpp>
#include <utility>

#include "utils.h"

#define TRACKER_TIMEOUT 15000

PeerRetriever::PeerRetriever(std::shared_ptr<Logger> logger, std::string peerId,
                             std::string announceUrl, std::string infoHash,
                             int port, const unsigned long fileSize)
    : fileSize(fileSize) {
  this->peerId = std::move(peerId);
  this->announceUrl = std::move(announceUrl);
  this->infoHash = std::move(infoHash);
  this->port = port;
}

tl::expected<std::vector<Peer *>, PeerRetrieverError>
PeerRetriever::retrievePeers(unsigned long bytesDownloaded) {
  fmt::println("inside PR PEERID {}", peerId);
  std::stringstream info;
  info << "Retrieving peers from " << announceUrl
       << " with the following parameters...\n";
  info << "info_hash: " << infoHash << "\n";
  info << "peer_id: " << peerId << "\n";
  info << "port: " << port << "\n";
  info << "uploaded: 0\n";
  info << "downloaded: " << bytesDownloaded << "\n";
  info << "left: " << (fileSize - bytesDownloaded) << "\n";
  info << "compact: 1"; // We are using compact mode for peer exchange

  cpr::Response res =
      cpr::Get(cpr::Url{announceUrl},
               cpr::Parameters{
                   {"info_hash",
                    std::string(hexDecode(
                        infoHash))}, // Assuming hexDecode is defined elsewhere
                   {"peer_id", std::string(peerId)},
                   {"port", std::to_string(port)},
                   {"uploaded", "0"},
                   {"downloaded", std::to_string(bytesDownloaded)},
                   {"left", std::to_string(fileSize - bytesDownloaded)},
                   {"compact", "1"}},
               cpr::Timeout{TRACKER_TIMEOUT});

  // Handle the response code
  if (res.status_code == 200) {
    // Decode the tracker response
    std::shared_ptr<bencoding::BItem> decodedResponse =
        bencoding::decode(res.text);
    auto peers = decodeResponse(res.text);

    // Check if peers were successfully decoded
    if (peers.has_value()) {
      return peers.value(); // Return the decoded peers
    } else {
      // Return error as tl::expected

      return tl::unexpected(
          PeerRetrieverError{"Failed to decode peers from response"});
    }
  } else {
    // Return error with the response status and text
    return tl::unexpected(
        PeerRetrieverError{"Failed to retrieve peers from tracker: " +
                           std::to_string(res.status_code) + ": " + res.text});
  }
}

tl::expected<std::vector<Peer *>, PeerRetrieverError>
PeerRetriever::decodeResponse(std::string response) {
  // LOG_F(INFO, "Decoding tracker response...");
  std::shared_ptr<bencoding::BItem> decodedResponse =
      bencoding::decode(response);

  std::shared_ptr<bencoding::BDictionary> responseDict =
      std::dynamic_pointer_cast<bencoding::BDictionary>(decodedResponse);
  std::shared_ptr<bencoding::BItem> peersValue =
      responseDict->getValue("peers");
  if (!peersValue) {
    return tl::unexpected(PeerRetrieverError{
        "Response returned by the tracker is not in the correct format. "
        "['peers' not found]"});
  }

  std::vector<Peer *> peers;

  if (typeid(*peersValue) == typeid(bencoding::BString)) {
    const int peerInfoSize = 6;
    std::string peersString =
        std::dynamic_pointer_cast<bencoding::BString>(peersValue)->value();

    if (peersString.length() % peerInfoSize != 0) {
      return tl::unexpected(PeerRetrieverError{
          "Received malformed 'peers' from tracker. ['peers' length needs to "
          "be divisible by 6]"});
    }

    const int peerNum = peersString.length() / peerInfoSize;
    for (int i = 0; i < peerNum; i++) {
      int offset = i * peerInfoSize;
      std::stringstream peerIp;
      peerIp << std::to_string((uint8_t)peersString[offset]) << ".";
      peerIp << std::to_string((uint8_t)peersString[offset + 1]) << ".";
      peerIp << std::to_string((uint8_t)peersString[offset + 2]) << ".";
      peerIp << std::to_string((uint8_t)peersString[offset + 3]);
      int peerPort = bytesToInt(peersString.substr(offset + 4, 2));
      Peer *newPeer = new Peer{peerIp.str(), peerPort};
      peers.push_back(newPeer);
    }
  }
  // Handles the second case where peer information is stored in a list
  else if (typeid(*peersValue) == typeid(bencoding::BList)) {
    std::shared_ptr<bencoding::BList> peerList =
        std::dynamic_pointer_cast<bencoding::BList>(peersValue);
    for (auto &item : *peerList) {
      // Casts each item to a dictionary
      std::shared_ptr<bencoding::BDictionary> peerDict =
          std::dynamic_pointer_cast<bencoding::BDictionary>(item);

      // Gets peer ip from the dictionary
      std::shared_ptr<bencoding::BItem> tempPeerIp = peerDict->getValue("ip");

      if (!tempPeerIp)
        return tl::unexpected(PeerRetrieverError{
            "Received malformed 'peers' from tracker. [Item does not contain "
            "key 'ip']"});

      std::string peerIp =
          std::dynamic_pointer_cast<bencoding::BString>(tempPeerIp)->value();
      // Gets peer port from the dictionary
      std::shared_ptr<bencoding::BItem> tempPeerPort =
          peerDict->getValue("port");
      if (!tempPeerPort) {
        return tl::unexpected(PeerRetrieverError{
            "Received malformed 'peers' from tracker. [Item does not contain "
            "key 'port']"});
      }
      int peerPort =
          (int)std::dynamic_pointer_cast<bencoding::BInteger>(tempPeerPort)
              ->value();
      Peer *newPeer = new Peer{peerIp, peerPort};
      peers.push_back(newPeer);
    }
  } else {
    return tl::unexpected(PeerRetrieverError{
        "Received malformed 'peers' from tracker. [Unknown type]"});
  }
  logger->log("Decode tracker response: SUCCESS");
  logger->log(fmt::format("Number of peers discovered: %zu", peers.size()));
  return peers;
}
