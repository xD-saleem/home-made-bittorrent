#include "torrent_parser.h"

#include <bencode/BDictionary.h>
#include <bencode/BItem.h>
#include <bencode/Decoder.h>
#include <bencode/bencoding.h>
#include <fmt/core.h>

#include <fstream>
#include <stdexcept>

torrent_parser::torrent_parser(const std::string& filePath) {
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

long torrent_parser::getFileSize() const { return 3; }

