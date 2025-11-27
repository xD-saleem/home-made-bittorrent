#include <openssl/sha.h>

#include <array>
#include <bitset>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

#include "Piece.h"

std::string sha1(const std::string& str) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(reinterpret_cast<const unsigned char*>(str.data()), str.size(), hash);

  std::string result;
  result.reserve(SHA_DIGEST_LENGTH * 2);
  for (unsigned char byte : hash) {
    result += std::format("{:02x}", byte);
  }

  return result;
}

std::string urlEncode(std::string_view value) {
  static constexpr auto kIsUnreserved = [](char c) noexcept {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '-' ||
           c == '_' || c == '.' || c == '~';
  };

  std::string result;
  result.reserve(value.size() * 3);

  for (unsigned char c : value) {
    if (kIsUnreserved(c)) {
      result.push_back(c);
    } else {
      // 2 digit uppercase hex
      result += std::format("%{:02X}", c);
    }
  }

  return result;
}

std::string hexDecode(const std::string& value) {
  int hash_length = value.length();
  std::string decoded_hex_string;
  for (int i = 0; i < hash_length; i += 2) {
    std::string byte = value.substr(i, 2);
    char c = static_cast<char>(strtol(byte.c_str(), nullptr, 16));
    decoded_hex_string.push_back(c);
  }
  return decoded_hex_string;
}

std::string hexEncode(const std::string& input) {
  static const char hexLetters[] = "0123456789ABCDEF";

  std::string output;
  output.reserve(input.length() * 2);
  for (auto c : input) {
    output.push_back('\\');
    output.push_back('x');
    output.push_back(hexLetters[c >> 4]);
    output.push_back(hexLetters[c & 15]);
  }
  return output;
}

bool doesHavePiece(const std::string& bitField, int index) {
  // Compute byte index and bit offset
  size_t byte_index = index / 8;
  int bit_offset = index % 8;

  // Safety check: make sure byteIndex is in bounds
  if (byte_index >= bitField.size()) return false;

  // Check the bit (most significant bit first)
  return (bitField[byte_index] & (1 << (7 - bit_offset))) != 0;
}

tl::expected<void, PieceError> setPiece(std::string& bitField, int index) {
  size_t byte_index = index / 8;
  int bit_offset = index % 8;

  // Safety check: make sure byteIndex is valid
  if (byte_index >= bitField.size()) {
    return tl::unexpected(PieceError("bit index is out of range"));
  }

  // Set the specific bit (most significant bit first)
  bitField[byte_index] |= (1 << (7 - bit_offset));
  return {};
}

int bytesToInt(std::string bytes) {
  std::string bin_str;
  int64_t byte_count = bytes.size();
  for (int i = 0; i < byte_count; i++)
    bin_str += std::bitset<8>(bytes[i]).to_string();
  return stoi(bin_str, nullptr, 2);
}

std::string formatTime(int64_t seconds) {
  if (seconds < 0) return "inf";

  std::string result;
  // compute h, m, s
  std::string h = std::to_string(seconds / 3600);
  std::string m = std::to_string((seconds % 3600) / 60);
  std::string s = std::to_string(seconds % 60);
  // add leading zero if needed
  std::string hh = std::string(2 - h.length(), '0') + h;
  std::string mm = std::string(2 - m.length(), '0') + m;
  std::string ss = std::string(2 - s.length(), '0') + s;
  // return mm:ss if hh is 00
  if (hh != "00") {
    result = hh + ':' + mm + ":" + ss;
  } else {
    result = mm + ":" + ss;
  }
  return result;
}
