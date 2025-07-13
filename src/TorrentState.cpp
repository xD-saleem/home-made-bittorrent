#include "TorrentState.h"

#include <fmt/core.h>

#include <string>
#include <utility>

TorrentState::~TorrentState() = default;

tl::expected<void, TorrentStateError> TorrentState::storeState(
    std::string hashinfo, std::string name) {
  auto val = databaseSvc_->insertOne(std::move(hashinfo), std::move(name));
  if (!val) {
    return tl::unexpected(TorrentStateError{"failed to save hashinfo"});
  }

  return {};
}

tl::expected<TorrentRecord, TorrentStateError> TorrentState::getState(
    std::string hashinfo) {
  auto val = databaseSvc_->getTorrent(std::move(hashinfo));
  return val.value();
}
