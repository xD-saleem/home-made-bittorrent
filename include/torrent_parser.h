#ifndef BITTORRENTCLIENT_TORRENT_PARSER_H
#define BITTORRENTCLIENT_TORRENT_PARSER_H

#include <bencode/BDictionary.h>

#include <string>

using byte = unsigned char;

class torrent_parser {
 private:
  std::shared_ptr<bencoding::BDictionary> root;

  // dependencies
  // TODO: Add dependencies here

 public:
  explicit torrent_parser(const std::string& filePath);
  long getFileSize() const;
  // long getPieceLength() const;
  // std::string getFileName() const;
  // std::string getAnnounce() const;
  // std::shared_ptr<bencoding::BItem> get(std::string key) const;
  // std::string getInfoHash() const;
  // std::vector<std::string> splitPieceHashes() const;
};

#endif  // #BITTORRENTCLIENT_TORRENT_PARSER_H
