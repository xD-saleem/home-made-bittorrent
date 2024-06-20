
#ifndef BITTORRENTCLIENT_TORRENTFILEPARSER_H
#define BITTORRENTCLIENT_TORRENTFILEPARSER_H

#include <bencode/BDictionary.h>

#include <string>
#include <tl/expected.hpp>
#include <vector>

struct TorrentFilParserError {
  std::string message;
};

using byte = unsigned char;

class TorrentFileParser {
 private:
  std::shared_ptr<bencoding::BDictionary> root;

 public:
  // Torrent file
  explicit TorrentFileParser(const std::string& filePath);

  long getFileSize() const;
  long getPieceLength() const;
  std::string getFileName() const;
  std::string getAnnounce() const;
  std::shared_ptr<bencoding::BItem> get(std::string key) const;
  std::string getInfoHash() const;
  tl::expected<std::vector<std::string>, TorrentFilParserError>
  splitPieceHashes() const;
};

#endif  // BITTORRENTCLIENT_TORRENTFILEPARSER_H
