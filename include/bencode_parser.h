#include <string>

#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

class BencodeParser {
 public:
  // Constructor
  BencodeParser();

  // Destructor
  ~BencodeParser();

  // parse method
  int parse(std::string bencode_string, int index);
};

#endif  // BENCODE_PARSER_H
