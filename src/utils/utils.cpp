#include <openssl/sha.h>

#include <array>
#include <bitset>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

using std::array;

namespace utils {
// NOLINTNEXTLINE(misc-use-internal-linkage)
std::string sha1(const std::string& str) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

  std::stringstream ss;
  for (unsigned char i : hash) {
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);
  }
  return ss.str();
}

// NOLINTNEXTLINE(misc-use-internal-linkage)
std::string urlEncode(std::string_view value) {
  static constexpr auto kIsUnreserved = [](char c) noexcept {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '-' ||
           c == '_' || c == '.' || c == '~';
  };

  // Estimate worst-case size: every char becomes "%XX" (3 bytes)
  std::string result;
  result.reserve(value.size() * 3);

  for (unsigned char c : value) {
    if (kIsUnreserved(c)) {
      result.push_back(c);
    } else {
      static constexpr array<char, 16> kHex = {'0', '1', '2', '3', '4', '5',
                                               '6', '7', '8', '9', 'A', 'B',
                                               'C', 'D', 'E', 'F'};
      result.push_back('%');
      result.push_back(kHex[c >> 4]);
      result.push_back(kHex[c & 0xF]);
    }
  }
  return result;
}

// NOLINTNEXTLINE(misc-use-internal-linkage)
std::string hexDecode(const std::string& value) {
  int hash_length = value.length();
  std::string decoded_hex_string;
  for (int i = 0; i < hash_length; i += 2) {
    std::string byte = value.substr(i, 2);
    char c =
        static_cast<char>(static_cast<int>(strtol(byte.c_str(), nullptr, 16)));
    decoded_hex_string.push_back(c);
  }
  return decoded_hex_string;
}

// NOLINTNEXTLINE(misc-use-internal-linkage)
std::string hexEncode(const std::string& input) {
  static const char kHexDigits[] = "0123456789ABCDEF";

  std::string output;
  output.reserve(input.length() * 2);
  for (unsigned char c : input) {
    output.push_back('\\');
    output.push_back('x');
    output.push_back(kHexDigits[c >> 4]);
    output.push_back(kHexDigits[c & 15]);
  }
  return output;
}

// NOLINTNEXTLINE(misc-use-internal-linkage)
int bytesToInt(std::string bytes) {
  std::string bin_str;
  int64_t byte_count = bytes.size();
  for (int i = 0; i < byte_count; i++)
    bin_str += std::bitset<8>(bytes[i]).to_string();
  return stoi(bin_str, nullptr, 2);
}

// NOLINTNEXTLINE(misc-use-internal-linkage)
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
}  // namespace utils
