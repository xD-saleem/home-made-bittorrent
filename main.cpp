
#include <arpa/inet.h>
#include <errno.h>
#include <fmt/core.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/torrent_info.hpp>
#include <optional>
#include <string>

struct TorrentFile {
  std::string announce;
  std::string name;
  int64_t piece_length;
  std::string pieces_hash;
  int64_t length;
  int64_t created_date;
  std::string info_hash;

  // std::string calculateInfoHash(const bencode::dict& info_data) {
  //   // Serialize the info dictionary
  //   std::ostringstream serialized_info;
  //   serialized_info << "d";  // Start of dictionary
  //
  //   for (const auto& [key, value] : info_data) {
  //     serialized_info << key << bencode::encode(value);
  //   }
  //
  //   serialized_info << "e";  // End of dictionary
  //
  //   // Compute the SHA-1 hash of the serialized info dictionary
  //   unsigned char hash[SHA_DIGEST_LENGTH];
  //   SHA1(reinterpret_cast<const unsigned
  //   char*>(serialized_info.str().c_str()),
  //        serialized_info.str().size(), hash);
  //
  //   // Convert the hash to a hexadecimal string
  //   std::ostringstream result;
  //   result << std::hex << std::uppercase;
  //   for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
  //     result << std::setw(2) << std::setfill('0') <<
  //     static_cast<int>(hash[i]);
  //   }
  //
  //   return result.str();
  // }
  //
  std::optional<std::string> build_tracker_url(std::string peer_id,
                                               std::string port) {
    {
      fmt::print("Building tracker url\n");
      fmt::print("Announce URL: {}\n", announce);
      if (announce.empty()) {
        return std::nullopt;
      }
      fmt::print("Building tracker url\n");

      std::string tracker_url = announce;
      tracker_url += "?info_hash=";
      tracker_url += info_hash;
      tracker_url += "&peer_id=";
      tracker_url += peer_id;
      tracker_url += "&port=";
      tracker_url += port;
      tracker_url += "&uploaded=0";
      tracker_url += "&downloaded=0";
      tracker_url += "&left=";
      tracker_url += std::to_string(length);
      tracker_url += "&compact=1";

      return tracker_url;
    }
  }

  std::optional<std::string> extractTrackerUrl(
      TorrentFile& file, const std::string& torrentFilePath) {
    libtorrent::error_code ec;

    std::string filename = "debian.torrent";

    libtorrent::torrent_info torrent(filename, ec);

    if (ec) {
      std::cerr << "Failed to parse torrent file: " << ec.message()
                << std::endl;
    }

    // Get the announce URL
    std::string announce = torrent.trackers().front().url;
    std::string name = torrent.name();
    std::string info_hash = torrent.info_hash().to_string();
    int64_t length = torrent.total_size();
    int64_t piece_length = torrent.piece_length();
    int64_t created_date = torrent.creation_date();

    libtorrent::piece_index_t piece_index =
        static_cast<libtorrent::piece_index_t>(0);
    auto pieces_hash = torrent.hash_for_piece(piece_index).to_string();

    fmt::print("Announce URL: {}\n", announce);
    fmt::print("Name: {}\n", name);
    fmt::print("Info Hash: {}\n", info_hash);
    fmt::print("Length: {}\n", length);
    fmt::print("Piece Length: {}\n", piece_length);
    fmt::print("Created Date: {}\n", created_date);

    auto trackers = torrent.trackers();
    fmt::print("Trackers: {}\n", trackers.size());

    file.announce = announce;
    file.name = name;
    file.piece_length = piece_length;
    file.pieces_hash = pieces_hash;
    file.length = length;
    file.created_date = created_date;
    file.info_hash = info_hash;

    auto option = build_tracker_url("3040040304304030232", "59432");
    if (option.has_value()) {
      return option.value();
    }
    return std::nullopt;
  };
};

struct Peer {
  std::string ip;
  int port;
};

int main() {
  TorrentFile torrent_file;
  std::string torrentFilePath = "debian.torrent";

  std::optional<std::string> url =
      torrent_file.extractTrackerUrl(torrent_file, torrentFilePath);

  if (!url.has_value()) {
    fmt::print("Failed to extract tracker url.\n");
  }
  fmt::print("Tracker URL: {}\n", url.value());
  int peerPort = 80;

  // Create a TCP socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
    return 1;
  }

  // Specify the address and port of the peer
  struct sockaddr_in peerAddrInfo;
  peerAddrInfo.sin_family = AF_INET;
  peerAddrInfo.sin_port = htons(peerPort);

  inet_pton(AF_INET, url.value().c_str(), &peerAddrInfo.sin_addr);

  // Connect to the peer
  if (connect(sockfd, (struct sockaddr*)&peerAddrInfo, sizeof(peerAddrInfo)) ==
      -1) {
    std::cerr << "Error connecting to peer: " << strerror(errno) << std::endl;
    close(sockfd);
    return 1;
  }

  char buffer[1024];
  fmt::print("Waiting for data from peer\n");

  // ready means the number of sockets that are ready to be read
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);
  const int SELECT_TIMEOUT = 100;
  struct timeval timeout;
  timeout.tv_sec = SELECT_TIMEOUT;
  timeout.tv_usec = 0;

  int ready = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

  if (ready == -1) {
    std::cerr << "Error waiting for data: " << strerror(errno) << std::endl;
    close(sockfd);
    return 1;
  } else if (ready == 0) {
    std::cout << "Timeout waiting for data" << std::endl;
    close(sockfd);
    return 1;
  }

  int bytesReceived = recv(sockfd, buffer, sizeof(buffer), 0);

  if (bytesReceived == -1) {
    std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
    close(sockfd);
    return 1;
  } else if (bytesReceived == 0) {
    std::cout << "Peer disconnected" << std::endl;
    close(sockfd);
    return 1;
  }

  fmt::print("Received {} bytes from peer\n", bytesReceived);

  // Close the socket
  close(sockfd);

  return 0;
};

