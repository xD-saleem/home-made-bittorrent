#include "PeerRetriever.h"

#include <bencode/bencoding.h>
#include <cpr/cpr.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <sys/types.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include <utility>

#include "Logger.h"
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
PeerRetriever::PeerRetriever(std::string peerId, std::string announceUrl,
                             std::string infoHash, int port,
                             const u_int64_t fileSize)
    : fileSize_(fileSize) {
  this->peerId_ = std::move(peerId);
  this->announceUrl_ = std::move(announceUrl);
  this->infoHash_ = std::move(infoHash);
  this->port_ = port;
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
std::vector<std::unique_ptr<Peer>> PeerRetriever::retrievePeers(
    u_int64_t bytesDownloaded) {
  std::stringstream info;
  info << "Retrieving peers from " << announceUrl_
       << " with the following parameters..." << std::endl;
  // Note that info hash will be URL-encoded by the cpr library
  info << "info_hash: " << infoHash_ << std::endl;
  info << "peer_id: " << peerId_ << std::endl;
  info << "port: " << port_ << std::endl;
  info << "uploaded: " << 0 << std::endl;
  info << "downloaded: " << std::to_string(bytesDownloaded) << std::endl;
  info << "left: " << std::to_string(fileSize_ - bytesDownloaded) << std::endl;
  info << "compact: " << std::to_string(1);

  cpr::Response res = cpr::Get(
      cpr::Url{announceUrl_},
      cpr::Parameters{{"info_hash", std::string(utils::hexDecode(infoHash_))},
                      {"peer_id", std::string(peerId_)},
                      {"port", std::to_string(port_)},
                      {"uploaded", std::to_string(0)},
                      {"downloaded", std::to_string(bytesDownloaded)},
                      {"left", std::to_string(fileSize_ - bytesDownloaded)},
                      {"compact", std::to_string(1)}},
      cpr::Timeout{TRACKER_TIMEOUT});

  // If response successfully retrieved
  if (res.status_code == 200) {
    std::shared_ptr<bencoding::BItem> decoded_response =
        bencoding::decode(res.text);

    tl::expected<std::vector<std::unique_ptr<Peer>>, PeerRetrieverError> peers =
        decodeResponse(res.text);

    if (!peers.has_value()) {
      Logger::log(fmt::format("Decoding tracker response: FAILED [ {} ]",
                              peers.error().message.c_str()));
      return std::vector<std::unique_ptr<Peer>>();
    }

    return std::move(peers.value());
  }
  Logger::log(fmt::format("Retrieving response from tracker: FAILED [ {}: {} ]",
                          res.status_code, res.text.c_str()));

  return std::vector<std::unique_ptr<Peer>>();
}

std::vector<std::unique_ptr<Peer>> PeerRetriever::retrieveSeedPeers(
    u_int64_t bytesDownloaded) {
  std::stringstream info;
  info << "Retrieving peers from " << announceUrl_
       << " with the following parameters..." << std::endl;
  // Note that info hash will be URL-encoded by the cpr library
  info << "info_hash: " << infoHash_ << std::endl;
  info << "peer_id: " << peerId_ << std::endl;
  info << "port: " << port_ << std::endl;
  info << "uploaded: " << 0 << std::endl;
  info << "downloaded: " << std::to_string(bytesDownloaded) << std::endl;
  info << "left: " << std::to_string(fileSize_ - bytesDownloaded) << std::endl;
  info << "compact: " << std::to_string(1);

  cpr::Response res = cpr::Get(
      cpr::Url{announceUrl_},
      cpr::Parameters{{"info_hash", std::string(utils::hexDecode(infoHash_))},
                      {"peer_id", std::string(peerId_)},
                      {"port", std::to_string(port_)},
                      {"uploaded", std::to_string(0)},
                      {"downloaded", std::to_string(bytesDownloaded)},
                      {"left", std::to_string(0)},
                      {"event", "completed"},
                      {"compact", std::to_string(1)}},
      cpr::Timeout{TRACKER_TIMEOUT});

  // If response successfully retrieved
  if (res.status_code == 200) {
    std::shared_ptr<bencoding::BItem> decoded_response =
        bencoding::decode(res.text);

    auto peers = decodeResponse(res.text);

    if (!peers.has_value()) {
      return std::vector<std::unique_ptr<Peer>>();
    }

    return std::move(peers.value());
  }
  Logger::log(fmt::format("Retrieving response from tracker: FAILED [ {}: {} ]",
                          res.status_code, res.text.c_str()));

  return std::vector<std::unique_ptr<Peer>>();
}

tl::expected<std::vector<std::unique_ptr<Peer>>, PeerRetrieverError>
PeerRetriever::decodeResponse(std::string response) {
  std::shared_ptr<bencoding::BItem> decoded_response =
      bencoding::decode(response);

  std::shared_ptr<bencoding::BDictionary> response_dict =
      std::dynamic_pointer_cast<bencoding::BDictionary>(decoded_response);
  std::shared_ptr<bencoding::BItem> peers_value =
      response_dict->getValue("peers");
  if (!peers_value) {
    return tl::unexpected(PeerRetrieverError{
        "Response returned by the tracker is not in the correct format. "
        "['peers' not found]"});
  }

  std::vector<std::unique_ptr<Peer>> peers;

  // Handles the first case where peer information is sent in a binary blob
  // (compact)
  if (typeid(*peers_value) == typeid(bencoding::BString)) {
    // Unmarshalls the peer information:
    // Detailed explanation can be found here:
    // https://blog.jse.li/posts/torrent/
    // Essentially, every 6 bytes represent a single peer with the first 4
    // bytes being the IP and the last 2 bytes being the port number.
    const int peer_info_size = 6;
    std::string peers_string =
        std::dynamic_pointer_cast<bencoding::BString>(peers_value)->value();

    if (peers_string.length() % peer_info_size != 0) {
      return tl::unexpected(PeerRetrieverError{
          "Received malformed 'peers' from tracker. ['peers' length needs to "
          "be divisible by 6]"});
    }

    const int peer_num = peers_string.length() / peer_info_size;
    for (int i = 0; i < peer_num; i++) {
      int offset = i * peer_info_size;
      std::stringstream peer_ip;
      peer_ip << std::to_string(static_cast<uint8_t>(peers_string[offset]))
              << ".";
      peer_ip << std::to_string(static_cast<uint8_t>(peers_string[offset + 1]))
              << ".";
      peer_ip << std::to_string(static_cast<uint8_t>(peers_string[offset + 2]))
              << ".";
      peer_ip << std::to_string(static_cast<uint8_t>(peers_string[offset + 3]));

      int peer_port = utils::bytesToInt(peers_string.substr(offset + 4, 2));

      std::unique_ptr<Peer> new_peer =
          std::make_unique<Peer>(Peer{.ip = peer_ip.str(), .port = peer_port});

      peers.push_back(std::move(new_peer));
    }
  }
  // Handles the second case where peer information is stored in a list
  else if (typeid(*peers_value) == typeid(bencoding::BList)) {
    std::shared_ptr<bencoding::BList> peer_list =
        std::dynamic_pointer_cast<bencoding::BList>(peers_value);
    for (auto& item : *peer_list) {
      // Casts each item to a dictionary
      std::shared_ptr<bencoding::BDictionary> peer_dict =
          std::dynamic_pointer_cast<bencoding::BDictionary>(item);

      // Gets peer ip from the dictionary
      std::shared_ptr<bencoding::BItem> temp_peer_ip =
          peer_dict->getValue("ip");

      if (!temp_peer_ip)
        return tl::unexpected(PeerRetrieverError{
            "Received malformed 'peers' from tracker. [Item does not contain "
            "key 'ip']"});

      std::string peer_ip =
          std::dynamic_pointer_cast<bencoding::BString>(temp_peer_ip)->value();
      // Gets peer port from the dictionary
      std::shared_ptr<bencoding::BItem> temp_peer_port =
          peer_dict->getValue("port");
      if (!temp_peer_port) {
        return tl::unexpected(PeerRetrieverError{
            "Received malformed 'peers' from tracker. [Item does not contain "
            "key 'port']"});
      }
      int peer_port = static_cast<int>(
          std::dynamic_pointer_cast<bencoding::BInteger>(temp_peer_port)
              ->value());

      std::unique_ptr<Peer> peer =
          std::make_unique<Peer>(Peer{.ip = peer_ip, .port = peer_port});
      peers.push_back(std::move(peer));
    }
  } else {
    return tl::unexpected(PeerRetrieverError{
        "Received malformed 'peers' from tracker. [Unknown type]"});
  }
  return peers;
}
