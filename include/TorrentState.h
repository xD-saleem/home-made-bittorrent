
#ifndef BITTORRENTCLIENT_TORRENTSTATE_H
#define BITTORRENTCLIENT_TORRENTSTATE_H

#include <cstdlib>
// #include <loguru/loguru.hpp>
#include <string>

#include "DatabaseService.h"
struct TorrentStateError {
  std::string message;
};

class TorrentState {
 private:
  std::shared_ptr<DatabaseService> databaseSvc;

 public:
  explicit TorrentState(std::shared_ptr<DatabaseService> dbSvc)
      : databaseSvc(dbSvc) {
    // LOG_F(INFO, "launching state");
    if (!databaseSvc) {  // Check if databaseSvc is null
      std::exit(1);
    }
  }

  tl::expected<void, TorrentStateError> storeState(std::string hashinfo,
                                                   std::string name);

  tl::expected<TorrentRecord, TorrentStateError> getState(std::string hashinfo);

  ~TorrentState();
};
#endif  // BITTORRENTCLIENT_TORRENTSTATE_H
