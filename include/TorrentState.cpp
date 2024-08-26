

#include "TorrentState.h"

#include <fmt/core.h>

#include <loguru/loguru.hpp>

TorrentState::TorrentState() { LOG_F(INFO, "launching state"); }

TorrentState::~TorrentState() = default;

std::string TorrentState::storeState() {
  fmt::println("OMG I AM A STATE: @@@@@@@@@@@@@@@");

  return "omg";
}
