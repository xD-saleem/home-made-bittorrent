
#ifndef BITTORRENTCLIENT_TORRENTFILEPARSER_H
#define BITTORRENTCLIENT_TORRENTFILEPARSER_H
#include <bencode/BDictionary.h>

#include <optional>
#include <string>
#include <vector>

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
  std::optional<std::vector<std::string>> splitPieceHashes() const;
};

#endif  // BITTORRENTCLIENT_TORRENTFILEPARSER_H
