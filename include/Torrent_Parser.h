#ifndef BITTORRENTCLIENT_TORRENT_PARSER_H
#define BITTORRENTCLIENT_TORRENT_PARSER_H

#include <bencode/BDictionary.h>

#include <optional>
#include <string>
#include <vector>

using byte = unsigned char;

class Torrent_Parser {
  // dependencies
  // TODO: Add dependencies here for dependency injection.
  //
 private:
  std::shared_ptr<bencoding::BDictionary> root;
  std::shared_ptr<bencoding::BItem> get(std::string key) const;

 public:
  explicit Torrent_Parser(const std::string& filePath);
  long getFileSize() const;
  long getPieceLength() const;
  std::string getFileName() const;
  std::string getAnnounce() const;
  std::string getInfoHash() const;
  std::optional<std::vector<std::string>> splitPieceHashes() const;

  // destory the object when it goes out of scope
  ~Torrent_Parser() = default;
};

#endif  // #BITTORRENTCLIENT_TORRENT_PARSER_H
