#include "TorrentFileParser.h"

#include <bencode/BItem.h>
#include <bencode/Decoder.h>
#include <bencode/bencoding.h>
#include <fmt/base.h>
#include <utils.h>

#include <cassert>
#include <fstream>
// #include <loguru/loguru.hpp>
#include <tl/expected.hpp>

#define HASH_LEN 20

TorrentFileParser::TorrentFileParser(const std::string &filePath) {
  // TODO have verification.
  std::ifstream fileStream(filePath, std::ifstream::binary);
  std::shared_ptr<bencoding::BItem> decodedTorrentFile =
      bencoding::decode(fileStream);
  std::shared_ptr<bencoding::BDictionary> rootDict =
      std::dynamic_pointer_cast<bencoding::BDictionary>(decodedTorrentFile);
  root = rootDict;
}

std::shared_ptr<bencoding::BItem>
TorrentFileParser::get(std::string key) const {
  std::shared_ptr<bencoding::BItem> value = root->getValue(key);
  return value;
}

std::string TorrentFileParser::getInfoHash() const {
  std::shared_ptr<bencoding::BItem> infoDictionary = get("info");
  std::string infoString = bencoding::encode(infoDictionary);
  std::string sha1Hash = sha1(infoString);
  return sha1Hash;
}

tl::expected<std::vector<std::string>, TorrentFileParserError>
TorrentFileParser::splitPieceHashes() const {
  std::shared_ptr<bencoding::BItem> piecesValue = get("pieces");

  if (!piecesValue) {
    // LOG_F(ERROR,
    // "Torrent file is malformed. [File does not contain key "
    // "'pieces']");
    return tl::unexpected(
        TorrentFileParserError{"Torrent file is malformed. [File does not "
                               "contain key 'pieces']"});
  }

  std::string pieces =
      std::dynamic_pointer_cast<bencoding::BString>(piecesValue)->value();

  std::vector<std::string> pieceHashes;

  assert(pieces.size() % HASH_LEN == 0);

  int piecesCount = (int)pieces.size() / HASH_LEN;
  pieceHashes.reserve(piecesCount);

  for (int i = 0; i < piecesCount; i++) {
    pieceHashes.push_back(pieces.substr(i * HASH_LEN, HASH_LEN));
  }

  return pieceHashes;
}

tl::expected<long, TorrentFileParserError>
TorrentFileParser::getFileSize() const {
  std::shared_ptr<bencoding::BItem> fileSizeItem = get("length");
  if (!fileSizeItem) {
    return tl::unexpected(
        TorrentFileParserError{"Torrent file is malformed. [File does not "
                               "contain key 'length']"});
  }

  return std::dynamic_pointer_cast<bencoding::BInteger>(fileSizeItem)->value();
}

tl::expected<long, TorrentFileParserError>
TorrentFileParser::getPieceLength() const {
  std::shared_ptr<bencoding::BItem> pieceLengthItem = get("piece length");
  if (!pieceLengthItem) {
    return tl::unexpected(TorrentFileParserError(
        "Torrent file is malformed. [File does not contain key 'piece "
        "length']"));
  }
  return std::dynamic_pointer_cast<bencoding::BInteger>(pieceLengthItem)
      ->value();
}

tl::expected<std::string, TorrentFileParserError>
TorrentFileParser::getFileName() const {
  std::shared_ptr<bencoding::BItem> filenameItem = get("name");
  if (!filenameItem) {
    return tl::unexpected(TorrentFileParserError(
        "Torrent file is malformed. [File does not contain key 'name']"));
  }
  return std::dynamic_pointer_cast<bencoding::BString>(filenameItem)->value();
}

tl::expected<std::string, TorrentFileParserError>
TorrentFileParser::getAnnounce() const {
  std::shared_ptr<bencoding::BItem> announceItem = get("announce");
  if (!announceItem) {
    return tl::unexpected(TorrentFileParserError(
        "Torrent file is malformed. [File does not contain key 'announce']"));
  }
  return std::dynamic_pointer_cast<bencoding::BString>(announceItem)->value();
}
