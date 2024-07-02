
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

  tl::expected<long, TorrentFileParserError> getFileSize() const;
  tl::expected<long, TorrentFileParserError> getPieceLength() const;
  tl::expected<std::string, TorrentFileParserError> getFileName() const;
  tl::expected<std::string, TorrentFileParserError> getAnnounce() const;
  std::shared_ptr<bencoding::BItem> get(std::string key) const;
  std::string getInfoHash() const;
  tl::expected<std::vector<std::string>, TorrentFileParserError>
  splitPieceHashes() const;
};

#endif  // BITTORRENTCLIENT_TORRENTFILEPARSER_H
