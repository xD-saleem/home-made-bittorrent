#include <fmt/core.h>

#include "bencode_parser.h"

int main() {
  BencodeParser parser;
  int result = parser.parse("d3:cow3:moo4:spam4:eggse", 0);
  fmt::print("Result: {}\n", result);
  return 0;
}

