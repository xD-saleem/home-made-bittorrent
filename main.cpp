
#include <fmt/core.h>
#include <openssl/sha.h>

#include <fstream>
#include <iomanip>
#include <optional>
#include <string>

#include "bencode_parser.hpp"

struct TorrentFile {
  std::string announce;
  std::string name;
  int64_t piece_length;
  std::string pieces_hash;
  int64_t length;
  int64_t created_date;
  std::string info_hash;

  std::string calculateInfoHash(const bencode::dict& info_data) {
    // Serialize the info dictionary
    std::ostringstream serialized_info;
    serialized_info << "d";  // Start of dictionary

    for (const auto& [key, value] : info_data) {
      serialized_info << key << bencode::encode(value);
    }

    serialized_info << "e";  // End of dictionary

    // Compute the SHA-1 hash of the serialized info dictionary
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(serialized_info.str().c_str()),
         serialized_info.str().size(), hash);

    // Convert the hash to a hexadecimal string
    std::ostringstream result;
    result << std::hex << std::uppercase;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
      result << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return result.str();
  }
  std::optional<std::string> build_tracker_url(std::string peer_id,
                                               std::string port) {
    {
      if (announce.empty()) {
        return std::nullopt;
      }

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
      tracker_url += length;
      tracker_url += "&compact=1";

      return tracker_url;
    }
  }
};

int main() {
  TorrentFile torrent_file;

  // Open the torrent file
  std::ifstream file("debian.torrent", std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file." << std::endl;
    return 1;
  }

  auto data = bencode::decode(file);

  auto announe = data["announce"];
  auto announe_value = std::get<bencode::string>(announe);

  auto comment = data["comment"];
  auto comment_value = std::get<bencode::string>(comment);

  auto created_by = data["created by"];
  auto created_by_value = std::get<bencode::string>(created_by);

  auto created_date = data["creation date"];
  auto created_date_value = std::get<bencode::integer>(created_date);

  auto length = data["info"]["length"];
  auto length_value = std::get<bencode::integer>(length);

  auto name = data["info"]["name"];
  auto name_value = std::get<bencode::string>(name);

  auto piece_length = data["info"]["piece length"];
  auto piece_length_value = std::get<bencode::integer>(piece_length);

  auto pieces = data["info"]["pieces"];
  auto pieces_hash = std::get<bencode::string>(pieces);

  auto info_hash = data["info"];
  auto info_hash_value = std::get<bencode::dict>(info_hash);

  std::string correct_info_hash =
      torrent_file.calculateInfoHash(std::ref(info_hash_value));

  torrent_file.announce = announe_value;
  torrent_file.name = name_value;
  torrent_file.piece_length = piece_length_value;
  torrent_file.pieces_hash = pieces_hash;
  torrent_file.length = length_value;
  torrent_file.created_date = created_date_value;
  torrent_file.info_hash = correct_info_hash;

  std::optional<std::string> url =
      torrent_file.build_tracker_url("fake_peer_id", "59432");

  if (!url.has_value()) {
    fmt::print("Failed to build tracker url.\n");
  } else {
    fmt::print("Tracker url: {}\n", url.value());
  }

  return 0;
}

