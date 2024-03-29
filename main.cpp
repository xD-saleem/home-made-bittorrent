
#include <fmt/core.h>

#include <fstream>
#include <optional>
#include <string>

#include "bencode_parser.hpp"

struct TorrentFile {
  std::string announce;
  std::string name;
  int64_t piece_length;
  std::string pieces;
  int64_t length;
  int64_t created_date;

  std::optional<std::string> build_tracker_url() {
    if (announce.empty()) {
      return std::nullopt;
    }

    std::string tracker_url = announce;
    tracker_url += "?info_hash=";
    tracker_url += "info_hash";
    tracker_url += "&peer_id=";
    tracker_url += "peer_id";
    tracker_url += "&port=";
    tracker_url += "port";
    tracker_url += "&uploaded=";
    tracker_url += "uploaded";
    tracker_url += "&downloaded=";
    tracker_url += "downloaded";
    tracker_url += "&left=";
    tracker_url += "left";
    tracker_url += "&compact=1";
    tracker_url += "&event=";
    tracker_url += "event";

    return tracker_url;
  }
};

int main() {
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
  auto pieces_value = std::get<bencode::string>(pieces);

  TorrentFile torrent_file;
  torrent_file.announce = announe_value;
  torrent_file.name = name_value;
  torrent_file.piece_length = piece_length_value;
  torrent_file.pieces = pieces_value;
  torrent_file.length = length_value;
  torrent_file.created_date = created_date_value;

  torrent_file.build_tracker_url();

  return 0;
}

