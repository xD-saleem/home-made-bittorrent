#include "TorrentState.h"

#include <fmt/core.h>

#include <string>

TorrentState::~TorrentState() = default;

tl::expected<void, TorrentStateError> TorrentState::storeState(
    std::string hashinfo, std::string name) {
  auto val = databaseSvc->insertOne(hashinfo, name);
  if (!val) {
    return tl::unexpected(TorrentStateError{"failed to save hashinfo"});
  }

  return {};
}

tl::expected<TorrentRecord, TorrentStateError> TorrentState::getState(
    std::string hashinfo) {
  auto val = databaseSvc->getTorrent(hashinfo);
  return val.value();
}
