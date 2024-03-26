#include "bencode_parser.h"

#include <fmt/core.h>

#include <string>

// Constructor
BencodeParser::BencodeParser() {}

// Destructor
BencodeParser::~BencodeParser() {}

// Parse a bencoded string

int BencodeParser::parse(std::string bencoded_string, int index) {
  fmt::print("Parsing bencoded string: {}\n", bencoded_string);
  // Parse the bencoded string
  return 0;
}

