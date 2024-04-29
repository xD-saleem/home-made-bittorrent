
#include "Torrent_Parser.h"

#include <bencode/BDictionary.h>
#include <bencode/BItem.h>
#include <bencode/Decoder.h>
#include <bencode/bencoding.h>
#include <fmt/core.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <cassert>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>

const int HASH_LEN = 20;

// Constructor
Torrent_Parser::Torrent_Parser(const std::string& filePath) {
  fmt::print("Reading torrent file: {}\n", filePath);

  std::ifstream fileStream(filePath, std::ifstream::binary);
  if (!fileStream.is_open()) {
    throw std::runtime_error("Could not open file");
  }

  std::shared_ptr<bencoding::BItem> decodedTorrentFile =
      bencoding::decode(fileStream);

  std::shared_ptr<bencoding::BDictionary> rootDict =
      std::dynamic_pointer_cast<bencoding::BDictionary>(decodedTorrentFile);

  root = rootDict;
}

// Calculate the SHA256 hash of the input string
std::string calculateSHA256(const std::string& input) {
  return std::string("info hash");
}

std::shared_ptr<bencoding::BItem> Torrent_Parser::get(std::string key) const {
  std::shared_ptr<bencoding::BItem> value = root->getValue(key);
  return value;
}
long Torrent_Parser::getFileSize() const {
  fmt::print("Getting file size\n");

  std::shared_ptr<bencoding::BItem> fileSizeItem = get("length");

  if (fileSizeItem == nullptr) {
    return -1;
  }

  return std::dynamic_pointer_cast<bencoding::BInteger>(fileSizeItem)->value();
}

std::string Torrent_Parser::getFileName() const {
  std::shared_ptr<bencoding::BItem> fileNameItem = get("name");

  if (fileNameItem == nullptr) {
    return "";
  }

  return std::dynamic_pointer_cast<bencoding::BString>(fileNameItem)->value();
}

std::string Torrent_Parser::getAnnounce() const {
  std::shared_ptr<bencoding::BItem> announceItem = get("announce");
  if (announceItem == nullptr) {
    return "";
  }
  return std::dynamic_pointer_cast<bencoding::BString>(announceItem)->value();
}

std::string Torrent_Parser::getInfoHash() const {
  std::shared_ptr<bencoding::BItem> infoItem = get("info");
  if (infoItem == nullptr) {
    return "";
  }

  std::string infoString = bencoding::encode(infoItem);

  return calculateSHA256(infoString);
}

long Torrent_Parser::getPieceLength() const {
  fmt::print("Getting piece length\n");

  return 3;
}

std::optional<std::vector<std::string>> Torrent_Parser::splitPieceHashes()
    const {
  std::shared_ptr<bencoding::BItem> piecesValue = get("pieces");

  if (!piecesValue) {
    return std::nullopt;
  }

  std::string pieces =
      std::dynamic_pointer_cast<bencoding::BString>(piecesValue)->value();

  std::vector<std::string> pieceHashes;

  assert(pieces.size() % HASH_LEN == 0);

  int piecesCount = pieces.size() / HASH_LEN;

  pieceHashes.reserve(piecesCount);

  for (int i = 0; i < piecesCount; i++) {
    pieceHashes.push_back(pieces.substr(i * HASH_LEN, HASH_LEN));
  }

  return pieceHashes;
}
