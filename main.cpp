// #include <iostream>
#include <loguru/loguru.hpp>
// #include <tl/expected.hpp>
//
#include "TorrentClient.h"
//
// int main(int argc, char* argv[]) {
//   if (__cplusplus == 202101L)
//     std::cout << "C++23";
//   else if (__cplusplus == 202002L)
//     std::cout << "C++20";
//   else if (__cplusplus == 201703L)
//     std::cout << "C++17";
//   else if (__cplusplus == 201402L)
//     std::cout << "C++14";
//   else if (__cplusplus == 201103L)
//     std::cout << "C++11";
//   else if (__cplusplus == 199711L)
//     std::cout << "C++98";
//   else
//     std::cout << "pre-standard C++." << __cplusplus;
//   std::cout << "\n";
//
//   std::string torrentFilePath = "debian.torrent";
//   loguru::init(argc, argv);
//
//   LOG_F(INFO, "Starting torrent client");
//
//   std::string filename = "debian.torrent";
//   std::string downloadDirectory = "./";
//   std::string downloadPath = downloadDirectory + filename;
//   std::string peerID = "peer_id";
//
//   TorrentClient torrentClient(20, true, "./");
//
//   LOG_F(INFO, "Downloading torrent file");
//
//   torrentClient.downloadFile(downloadPath, downloadDirectory);
//
//   LOG_F(INFO, "Downloaded torrent file successfully");
//   return 0;
// };
//
//
#include <fmt/core.h>

#include <fstream>
#include <iostream>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_info.hpp>
#include <map>
#include <sstream>
#include <string>

// std::map<std::string, std::string> parseMagnetURI(
//     const std::string& magnetURI) {
//   std::map<std::string, std::string> parsedData;
//
//   // Ensure that the URI starts with "magnet:?"
//   if (magnetURI.substr(0, 8) != "magnet:?") {
//     std::cerr << "Invalid Magnet URI" << std::endl;
//     return parsedData;
//   }
//
//   // Remove the "magnet:?" prefix
//   std::string uriContent = magnetURI.substr(8);
//
//   // Split the URI by "&"
//   std::stringstream ss(uriContent);
//   std::string segment;
//
//   while (std::getline(ss, segment, '&')) {
//     size_t equalPos = segment.find('=');
//     if (equalPos != std::string::npos) {
//       std::string key = segment.substr(0, equalPos);
//       std::string value = segment.substr(equalPos + 1);
//       parsedData[key] = value;
//     }
//   }
//
//   return parsedData;
// }

// int main() {
//   std::string magnetURI =
//       "magnet:?xt=urn:btih:D8A6EA932113D6C1DF5376224C22EA398A482E37&dn=%"
//       "5BSubsPlease%5D+One+Piece+-+1109+%281080p%29+%5B32DA2FE4%5D.mkv&tr=http%"
//       "3A%2F%2Fp4p.arenabg.com%3A1337%2Fannounce&tr=udp%3A%2F%2F47.ip-51-68-"
//       "199.eu%3A6969%2Fannounce&tr=udp%3A%2F%2F9.rarbg.me%3A2780%2Fannounce&tr="
//       "udp%3A%2F%2F9.rarbg.to%3A2710%2Fannounce&tr=udp%3A%2F%2F9.rarbg.to%"
//       "3A2730%2Fannounce&tr=udp%3A%2F%2F9.rarbg.to%3A2920%2Fannounce&tr=udp%3A%"
//       "2F%2Fopen.stealth.si%3A80%2Fannounce&tr=udp%3A%2F%2Fopentracker.i2p."
//       "rocks%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969%"
//       "2Fannounce&tr=udp%3A%2F%2Ftracker.cyberia.is%3A6969%2Fannounce&tr=udp%"
//       "3A%2F%2Ftracker.dler.org%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker."
//       "internetwarriors.net%3A1337%2Fannounce&tr=udp%3A%2F%2Ftracker.leechers-"
//       "paradise.org%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.openbittorrent."
//       "com%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr="
//       "udp%3A%2F%2Ftracker.pirateparty.gr%3A6969%2Fannounce&tr=udp%3A%2F%"
//       "2Ftracker.tiny-vps.com%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.torrent."
//       "eu.org%3A451%2Fannounce";
//
//   libtorrent::settings_pack settings;
//   settings.set_int(libtorrent::settings_pack::alert_mask,
//                    libtorrent::alert::status_notification |
//                        libtorrent::alert::error_notification);
//
//   libtorrent::session ses(settings);
//
//   libtorrent::add_torrent_params params;
//   libtorrent::error_code ec;
//   libtorrent::parse_magnet_uri(magnetURI, params, ec);
//   if (ec) {
//     std::cerr << "Error parsing magnet URI: " << ec.message() << std::endl;
//     return 1;
//   }
//
//   params.save_path = "./";
//
//   libtorrent::torrent_handle th = ses.add_torrent(params);
//
//   // Wait until we have the metadata
//   bool metadata_received = false;
//   while (!metadata_received) {
//     std::vector<libtorrent::alert*> alerts;
//     ses.pop_alerts(&alerts);
//     for (libtorrent::alert* a : alerts) {
//       auto at =
//       libtorrent::alert_cast<libtorrent::metadata_received_alert>(a); if (at)
//       {
//         metadata_received = true;
//         break;
//       }
//     }
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));
//   }
//
//   auto tinfo = th.torrent_file();
//
//   long totalSize = tinfo->total_size();
//   long pieceSize = tinfo->piece_length();
//   int numPieces = tinfo->num_pieces();
//   std::string fileName = tinfo->name();
//   std::string announce = tinfo->trackers()[0].url;
//   std::string infoHash = tinfo->info_hash().to_string();
//
//   fmt::print("total size: {}\n", totalSize);
//   fmt::print("piece size: {}\n", pieceSize);
//   fmt::print("file name: {}\n", fileName);
//   fmt::print("announce: {}\n", announce);
//   fmt::print("info hash: {}\n", infoHash);
//   fmt::print("num pieces: {}\n", numPieces);
//
//   return 0;
// }

int main() {
  std::string magnet_uri =
      "magnet:?xt=urn:btih:AB2B5FD90939DF1C2A8BF3E1198228155420296F&dn=One%"
      "20Piece%20-%201107%20(720p)(3D15C094)-Erai-raws%5BTGx%5D&tr=udp%3A%2F%"
      "2Ftracker.opentrackr.org%3A1337&tr=udp%3A%2F%2Fopen.stealth.si%3A80%"
      "2Fannounce&tr=udp%3A%2F%2Ftracker.torrent.eu.org%3A451%2Fannounce&tr="
      "udp%3A%2F%2Ftracker.bittor.pw%3A1337%2Fannounce&tr=udp%3A%2F%2Fpublic."
      "popcorn-tracker.org%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.dler.org%"
      "3A6969%2Fannounce&tr=udp%3A%2F%2Fexodus.desync.com%3A6969&tr=udp%3A%2F%"
      "2Fopen.demonii.com%3A1337%2Fannounce";

  // Create a session
  libtorrent::settings_pack settings;
  settings.set_int(libtorrent::settings_pack::alert_mask,
                   libtorrent::alert_category::status);
  libtorrent::session ses(settings);

  // Add the magnet link to the session
  libtorrent::add_torrent_params atp = libtorrent::parse_magnet_uri(magnet_uri);
  atp.save_path = "./";  // Where to save the downloaded files
  libtorrent::torrent_handle th = ses.add_torrent(atp);

  // Wait until we have the metadata
  bool metadata_received = false;
  while (!metadata_received) {
    std::vector<libtorrent::alert*> alerts;
    ses.pop_alerts(&alerts);

    for (libtorrent::alert* a : alerts) {
      if (libtorrent::alert_cast<libtorrent::metadata_received_alert>(a)) {
        metadata_received = true;
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  // Access the torrent_info
  std::shared_ptr<const libtorrent::torrent_info> tinfo = th.torrent_file();

  if (!tinfo) {
    std::cerr << "Failed to retrieve torrent info" << std::endl;
    return 1;
  }

  // std::string announceUrl = torrentFileParser.getAnnounce().value();
  // long fileSize = torrentFileParser.getFileSize().value();
  // const std::string infoHash = torrentFileParser.getInfoHash();
  // std::string filename = torrentFileParser.getFileName().value();

  auto infoHash = tinfo->info_hash().to_string();
  auto pieces = tinfo->piece_size();
  int totalSize = tinfo->total_size();
  std::string announeUrl = tinfo->trackers().at(0).url;

  fmt::println("Total size: {}", totalSize);
  fmt::println("announe url: {}", announeUrl);
  fmt::println("info hash: {}", infoHash);
  fmt::println(

  // tinfo->hash_for_piece(piece_index_t index)
  //     // Create a .torrent file from the torrent_info
  //     libtorrent::create_torrent ct(*tinfo);
  //
  // std::vector<char> torrent;
  // libtorrent::bencode(std::back_inserter(torrent), ct.generate());
  //
  // std::string torrentFilePath = "test.torrent";
  // std::ofstream outfile(torrentFilePath, std::ios::binary);
  // outfile.write(torrent.data(), torrent.size());
  // outfile.close();
  //
  // fmt::print("Torrent file saved to {}\n", torrentFilePath);
  //
  // std::string downloadDirectory = "./";
  // std::string downloadPath = downloadDirectory + torrentFilePath;
  // std::string peerID = "peer_id";
  //
  // TorrentClient torrentClient(20, true, "./");
  //
  // LOG_F(INFO, "Downloading torrent file");
  //
  // torrentClient.downloadFile(downloadPath, downloadDirectory);
  //
  LOG_F(INFO, "Downloaded torrent file successfully");
  return 0;
};
