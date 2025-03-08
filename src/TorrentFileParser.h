
#ifndef BITTORRENTCLIENT_TORRENTFILEPARSER_H
#define BITTORRENTCLIENT_TORRENTFILEPARSER_H

#include <bencode/BDictionary.h>

#include <string>
#include <tl/expected.hpp>
#include <vector>

struct TorrentFileParserError {
  std::string message;
};

using byte = unsigned char;

class TorrentFileParser {
 private:
  std::shared_ptr<bencoding::BDictionary> root;

 public:
  // Torrent file
  explicit TorrentFileParser(const std::string& filePath);

  [[nodiscard]] tl::expected<long, TorrentFileParserError> getFileSize() const;
  [[nodiscard]] tl::expected<long, TorrentFileParserError> getPieceLength()
      const;
  [[nodiscard]] tl::expected<std::string, TorrentFileParserError> getFileName()
      const;
  [[nodiscard]] tl::expected<std::string, TorrentFileParserError> getAnnounce()
      const;
  [[nodiscard]] std::shared_ptr<bencoding::BItem> get(std::string key) const;
  [[nodiscard]] std::string getInfoHash() const;
  [[nodiscard]] tl::expected<std::vector<std::string>, TorrentFileParserError>
  splitPieceHashes() const;
};

#endif  // BITTORRENTCLIENT_TORRENTFILEPARSER_H
