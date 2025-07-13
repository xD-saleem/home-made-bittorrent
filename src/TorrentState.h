
#ifndef BITTORRENTCLIENT_TORRENTSTATE_H
#define BITTORRENTCLIENT_TORRENTSTATE_H

#include <cstdlib>
#include <string>
#include <utility>

#include "DatabaseService.h"

struct TorrentStateError {
  std::string message;
};

class TorrentState {
 private:
  std::shared_ptr<DatabaseService> databaseSvc_;

 public:
  explicit TorrentState(std::shared_ptr<DatabaseService> dbSvc)
      : databaseSvc_(std::move(dbSvc)) {
    if (!databaseSvc_) {
      std::exit(1);
    }
  }

  tl::expected<void, TorrentStateError> storeState(std::string hashinfo,
                                                   std::string name);

  tl::expected<TorrentRecord, TorrentStateError> getState(std::string hashinfo);

  ~TorrentState();
};
#endif  // BITTORRENTCLIENT_TORRENTSTATE_H
