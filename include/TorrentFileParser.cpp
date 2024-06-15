#include "TorrentFileParser.h"

#include <bencode/BItem.h>
#include <bencode/Decoder.h>
#include <bencode/bencoding.h>
#include <utils.h>

#include <cassert>
#include <fstream>
#include <loguru/loguru.hpp>
#include <optional>
#include <stdexcept>

#define HASH_LEN 20

TorrentFileParser::TorrentFileParser(const std::string& filePath) {
  // TODO have verification.
  LOG_F(INFO, "Parsing Torrent file %s...", filePath.c_str());
  std::ifstream fileStream(filePath, std::ifstream::binary);
  std::shared_ptr<bencoding::BItem> decodedTorrentFile =
      bencoding::decode(fileStream);
  std::shared_ptr<bencoding::BDictionary> rootDict =
      std::dynamic_pointer_cast<bencoding::BDictionary>(decodedTorrentFile);
  root = rootDict;
  LOG_F(INFO, "Parse Torrent file: SUCCESS");
}

std::shared_ptr<bencoding::BItem> TorrentFileParser::get(
    std::string key) const {
  std::shared_ptr<bencoding::BItem> value = root->getValue(key);
  return value;
}

std::string TorrentFileParser::getInfoHash() const {
  std::shared_ptr<bencoding::BItem> infoDictionary = get("info");
  std::string infoString = bencoding::encode(infoDictionary);
  std::string sha1Hash = sha1(infoString);
  return sha1Hash;
}

std::optional<std::vector<std::string>> TorrentFileParser::splitPieceHashes()
    const {
  std::shared_ptr<bencoding::BItem> piecesValue = get("pieces");

  if (!piecesValue) {
    LOG_F(ERROR,
          "Torrent file is malformed. [File does not contain key "
          "'pieces']");
    return std::nullopt;
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

long TorrentFileParser::getFileSize() const {
  std::shared_ptr<bencoding::BItem> fileSizeItem = get("length");
  if (!fileSizeItem)
    throw std::runtime_error(
        "Torrent file is malformed. [File does not contain key 'length']");
  long fileSize =
      std::dynamic_pointer_cast<bencoding::BInteger>(fileSizeItem)->value();
  return fileSize;
}

long TorrentFileParser::getPieceLength() const {
  std::shared_ptr<bencoding::BItem> pieceLengthItem = get("piece length");
  if (!pieceLengthItem)
    throw std::runtime_error(
        "Torrent file is malformed. [File does not contain key 'piece "
        "length']");
  long pieceLength =
      std::dynamic_pointer_cast<bencoding::BInteger>(pieceLengthItem)->value();
  return pieceLength;
}

std::string TorrentFileParser::getFileName() const {
  std::shared_ptr<bencoding::BItem> filenameItem = get("name");
  if (!filenameItem)
    throw std::runtime_error(
        "Torrent file is malformed. [File does not contain key 'name']");
  std::string filename =
      std::dynamic_pointer_cast<bencoding::BString>(filenameItem)->value();
  return filename;
}

std::string TorrentFileParser::getAnnounce() const {
  std::shared_ptr<bencoding::BItem> announceItem = get("announce");
  if (!announceItem)
    throw std::runtime_error(
        "Torrent file is malformed. [File does not contain key 'announce']");
  std::string announce =
      std::dynamic_pointer_cast<bencoding::BString>(announceItem)->value();
  return announce;
}
