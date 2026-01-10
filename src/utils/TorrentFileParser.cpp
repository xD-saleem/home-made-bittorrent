#include "TorrentFileParser.h"

#include <bencode/BItem.h>
#include <bencode/Decoder.h>
#include <bencode/bencoding.h>
#include <fmt/base.h>
#include <utils/utils.h>

#include <cassert>
#include <cstdint>
#include <fstream>
#include <tl/expected.hpp>

constexpr int kHashLen = 20;

TorrentFileParser::TorrentFileParser(const std::string& filePath) {
  std::ifstream file_stream(filePath, std::ifstream::binary);
  std::shared_ptr<bencoding::BItem> decoded_torrent_file =
      bencoding::decode(file_stream);
  std::shared_ptr<bencoding::BDictionary> root_dict =
      std::dynamic_pointer_cast<bencoding::BDictionary>(decoded_torrent_file);
  root_ = root_dict;
}

std::shared_ptr<bencoding::BItem> TorrentFileParser::get(
    std::string key) const {
  std::shared_ptr<bencoding::BItem> value = root_->getValue(key);
  return value;
}

std::string TorrentFileParser::getInfoHash() const {
  std::shared_ptr<bencoding::BItem> info_dictionary = get("info");
  std::string info_string = bencoding::encode(info_dictionary);
  std::string sha1_hash = utils::sha1(info_string);
  return sha1_hash;
}

tl::expected<std::vector<std::string>, TorrentFileParserError>
TorrentFileParser::splitPieceHashes() const {
  std::shared_ptr<bencoding::BItem> pieces_value = get("pieces");

  if (!pieces_value) {
    return tl::unexpected(
        TorrentFileParserError{"Torrent file is malformed. [File does not "
                               "contain key 'pieces']"});
  }

  std::string pieces =
      std::dynamic_pointer_cast<bencoding::BString>(pieces_value)->value();

  std::vector<std::string> piece_hashes;

  assert(pieces.size() % kHashLen == 0);

  int pieces_count = static_cast<int>(pieces.size()) / kHashLen;
  piece_hashes.reserve(pieces_count);

  for (int i = 0; i < pieces_count; i++) {
    piece_hashes.push_back(pieces.substr(i * kHashLen, kHashLen));
  }

  return piece_hashes;
}

tl::expected<int64_t, TorrentFileParserError> TorrentFileParser::getFileSize()
    const {
  std::shared_ptr<bencoding::BItem> file_size_item = get("length");
  if (!file_size_item) {
    return tl::unexpected(
        TorrentFileParserError{"Torrent file is malformed. [File does not "
                               "contain key 'length']"});
  }

  return std::dynamic_pointer_cast<bencoding::BInteger>(file_size_item)
      ->value();
}

tl::expected<int64_t, TorrentFileParserError>
TorrentFileParser::getPieceLength() const {
  std::shared_ptr<bencoding::BItem> piece_length_item = get("piece length");
  if (!piece_length_item) {
    return tl::unexpected(TorrentFileParserError{
        "Torrent file is malformed. [File does not contain key 'piece "
        "length']"});
  }
  return std::dynamic_pointer_cast<bencoding::BInteger>(piece_length_item)
      ->value();
}

tl::expected<std::string, TorrentFileParserError>
TorrentFileParser::getFileName() const {
  std::shared_ptr<bencoding::BItem> filename_item = get("name");
  if (!filename_item) {
    return tl::unexpected(TorrentFileParserError{
        "Torrent file is malformed. [File does not contain key 'name']"});
  }
  return std::dynamic_pointer_cast<bencoding::BString>(filename_item)->value();
}

tl::expected<std::string, TorrentFileParserError>
TorrentFileParser::getAnnounce() const {
  std::shared_ptr<bencoding::BItem> announce_item = get("announce");
  if (!announce_item) {
    return tl::unexpected(TorrentFileParserError{
        "Torrent file is malformed. [File does not contain key 'announce']"});
  }
  return std::dynamic_pointer_cast<bencoding::BString>(announce_item)->value();
}
