#include <openssl/sha.h>

#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <loguru/loguru.hpp>
#include <sstream>
#include <string>

std::string sha1(const std::string& str) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

  std::stringstream ss;
  for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<int>(hash[i]);
  }
  return ss.str();
}

std::string urlEncode(const std::string& value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (char c : value) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char)c);
    escaped << std::nouppercase;
  }

  return escaped.str();
}

std::string hexDecode(const std::string& value) {
  int hashLength = value.length();
  std::string decodedHexString;
  for (int i = 0; i < hashLength; i += 2) {
    std::string byte = value.substr(i, 2);
    char c = (char)(int)strtol(byte.c_str(), nullptr, 16);
    decodedHexString.push_back(c);
  }
  return decodedHexString;
}

std::string hexEncode(const std::string& input) {
  static const char hexDigits[] = "0123456789ABCDEF";

  std::string output;
  output.reserve(input.length() * 2);
  for (unsigned char c : input) {
    output.push_back('\\');
    output.push_back('x');
    output.push_back(hexDigits[c >> 4]);
    output.push_back(hexDigits[c & 15]);
  }
  return output;
}

bool hasPiece(const std::string& bitField, int index) {
  int byteIndex = floor(index / 8);
  int offset = index % 8;
  return (bitField[byteIndex] >> (7 - offset) & 1) != 0;
}

void setPiece(std::string& bitField, int index) {
  int byteIndex = floor(index / 8);
  int offset = index % 8;
  bitField[byteIndex] |= (1 << (7 - offset));
}

int bytesToInt(std::string bytes) {
  // FIXME: Use bitwise operation to convert
  std::string binStr;
  long byteCount = bytes.size();
  for (int i = 0; i < byteCount; i++)
    binStr += std::bitset<8>(bytes[i]).to_string();
  return stoi(binStr, 0, 2);
}

std::string formatTime(long seconds) {
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
  if (hh.compare("00") != 0) {
    result = hh + ':' + mm + ":" + ss;
  } else {
    result = mm + ":" + ss;
  }
  return result;
}
