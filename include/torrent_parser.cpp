#include "torrent_parser.h"

// #include <fmt/core.h>
// #include <lib/bencode/BDictionary.h>

#include <fstream>
#include <iostream>
#include <vector>

// initialize the root shared pointer with a BDictionary object
torrent_parser::torrent_parser(const std::string& filePath) {
  // fmt::print("Reading torrent file: {}\n", filePath);
  // std::ifstream file(filePath, std::ios::binary);
  //
  // if (!file.is_open()) {
  //   // Panic
  //   throw std::runtime_error("Could not open file");
  // }
  //
  // std::vector<byte> buffer(std::istreambuf_iterator<char>(file), {});
  //
  // root = std::make_shared<bencoding::BDictionary>(buffer);
}

long torrent_parser::getFileSize() const { return 3; }

