#include "TorrentHandler.h"

#include <gtest/gtest.h>

#include "string"

struct MagnetTorrent {};

std::string TorrentHandler::handle(const std::string& magneticLink) {
  return "TODO";
}

std::string TorrentHandler::parseMagnet(const std::string& magneticLink) {
  return "file";
}

