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

/**
 * Constructor of the class PeerRetriever. Takes in the URL as specified by the
 * value of announce in the Torrent file, the info hash of the file, as well as
 * a port number.
 * @param announceURL: the HTTP URL to the tracker.
 * @param infoHash: the info hash of the Torrent file.
 * @param port: the TCP port this client listens on.
 * @param fileSize: the size of the file to be downloaded in bytes.
 */
PeerRetriever::PeerRetriever(std::shared_ptr<Logger> logger, std::string peerId,
                             std::string announceUrl, std::string infoHash,
                             int port, const unsigned long fileSize)
    : fileSize(fileSize) {
  this->peerId = std::move(peerId);
  this->announceUrl = std::move(announceUrl);
  this->infoHash = std::move(infoHash);
  this->port = port;
  this->logger = logger;
}

/**
 * Retrieves the list of peers from the URL specified by the 'announce'
 * property. The list of parameters and their descriptions are as follows (found
 * on this page https://markuseliasson.se/article/bittorrent-in-python/):
 * - info_hash: the SHA1 hash of the info dict found in the .torrent.
 * - peer_id: a unique ID generated for this client.
 * - uploaded: the total number of bytes uploaded.
 * - downloaded: the total number of bytes downloaded.
 * - left: the number of bytes left to download for this client.
 * - port: the TCP port this client listens on.
 * - compact: whether or not the client accepts a compacted list of peers or
 * not.
 * @return a vector that contains the information of all peers.
 */
std::vector<Peer *> PeerRetriever::retrievePeers(
    unsigned long bytesDownloaded) {
  std::stringstream info;
  info << "Retrieving peers from " << announceUrl
       << " with the following parameters..." << std::endl;
  // Note that info hash will be URL-encoded by the cpr library
  info << "info_hash: " << infoHash << std::endl;
  info << "peer_id: " << peerId << std::endl;
  info << "port: " << port << std::endl;
  info << "uploaded: " << 0 << std::endl;
  info << "downloaded: " << std::to_string(bytesDownloaded) << std::endl;
  info << "left: " << std::to_string(fileSize - bytesDownloaded) << std::endl;
  info << "compact: " << std::to_string(1);

  cpr::Response res = cpr::Get(
      cpr::Url{announceUrl},
      cpr::Parameters{{"info_hash", std::string(hexDecode(infoHash))},
                      {"peer_id", std::string(peerId)},
                      {"port", std::to_string(port)},
                      {"uploaded", std::to_string(0)},
                      {"downloaded", std::to_string(bytesDownloaded)},
                      {"left", std::to_string(fileSize - bytesDownloaded)},
                      {"compact", std::to_string(1)}},
      cpr::Timeout{TRACKER_TIMEOUT});

  // If response successfully retrieved
  if (res.status_code == 200) {
    std::shared_ptr<bencoding::BItem> decodedResponse =
        bencoding::decode(res.text);

    auto peers = decodeResponse(res.text);

    if (!peers.has_value()) {
      logger->log(fmt::format("Decoding tracker response: FAILED [ {} ]",
                              peers.error().message.c_str()));
      return std::vector<Peer *>();
    }

    return peers.value();
  } else {
    logger->log(
        fmt::format("Retrieving response from tracker: FAILED [ {}: {} ]",
                    res.status_code, res.text.c_str()));
  }
  return std::vector<Peer *>();
}

std::vector<Peer *> PeerRetriever::retrieveSeedPeers(
    unsigned long bytesDownloaded) {
  std::stringstream info;
  info << "Retrieving peers from " << announceUrl
       << " with the following parameters..." << std::endl;
  // Note that info hash will be URL-encoded by the cpr library
  info << "info_hash: " << infoHash << std::endl;
  info << "peer_id: " << peerId << std::endl;
  info << "port: " << port << std::endl;
  info << "uploaded: " << 0 << std::endl;
  info << "downloaded: " << std::to_string(bytesDownloaded) << std::endl;
  info << "left: " << std::to_string(fileSize - bytesDownloaded) << std::endl;
  info << "compact: " << std::to_string(1);

  // LOG_F(INFO, "%s", info.str().c_str());

  cpr::Response res =
      cpr::Get(cpr::Url{announceUrl},
               cpr::Parameters{{"info_hash", std::string(hexDecode(infoHash))},
                               {"peer_id", std::string(peerId)},
                               {"port", std::to_string(port)},
                               {"uploaded", std::to_string(0)},
                               {"downloaded", std::to_string(bytesDownloaded)},
                               {"left", std::to_string(0)},
                               {"event", "completed"},
                               {"compact", std::to_string(1)}},
               cpr::Timeout{TRACKER_TIMEOUT});

  // If response successfully retrieved
  if (res.status_code == 200) {
    std::shared_ptr<bencoding::BItem> decodedResponse =
        bencoding::decode(res.text);

    auto peers = decodeResponse(res.text);

    if (!peers.has_value()) {
      return std::vector<Peer *>();
    }

    return peers.value();
  } else {
    logger->log(
        fmt::format("Retrieving response from tracker: FAILED [ {}: {} ]",
                    res.status_code, res.text.c_str()));
  }

  return std::vector<Peer *>();
}

tl::expected<std::vector<Peer *>, PeerRetrieverError>
PeerRetriever::decodeResponse(std::string response) {
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

  // Handles the first case where peer information is sent in a binary blob
  // (compact)
  if (typeid(*peersValue) == typeid(bencoding::BString)) {
    // Unmarshalls the peer information:
    // Detailed explanation can be found here:
    // https://blog.jse.li/posts/torrent/
    // Essentially, every 6 bytes represent a single peer with the first 4
    // bytes being the IP and the last 2 bytes being the port number.
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
  return peers;
}
