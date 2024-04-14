#ifndef BITTORRENTCLIENT_TORRENT_PARSER_H
#define BITTORRENTCLIENT_TORRENT_PARSER_H

#include <bencode/BDictionary.h>

#include <optional>
#include <string>
#include <vector>

using byte = unsigned char;

class torrent_parser {
  // dependencies
  // TODO: Add dependencies here for dependency injection.
  //
 private:
  std::shared_ptr<bencoding::BDictionary> root;
  std::shared_ptr<bencoding::BItem> get(std::string key) const;

 public:
  explicit torrent_parser(const std::string& filePath);
  long getFileSize() const;
  long getPieceLength() const;
  std::string getFileName() const;
  std::string getAnnounce() const;
  std::string getInfoHash() const;
  std::optional<std::vector<std::string>> splitPieceHashes() const;
};

#endif  // #BITTORRENTCLIENT_TORRENT_PARSER_H
