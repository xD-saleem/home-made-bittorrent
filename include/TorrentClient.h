
#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <string>

#include "PeerConnection.h"
#include "PeerRetriever.h"
#include "SharedQueue.h"

struct TorrentClientError {
  std::string message;
};

class TorrentClient {
 private:
  // deps
  // TorrentState torrentState;

  const int threadNum;
  std::string peerId;
  SharedQueue<Peer*> queue;
  std::vector<std::thread> threadPool;
  std::vector<PeerConnection*> connections;

 public:
  explicit TorrentClient(int threadNum = 5, bool enableLogging = true,
                         std::string logFilePath = "logs/client.log");
  ~TorrentClient();
  void terminate();
  void downloadFile(const std::string& torrentFilePath,
                    const std::string& downloadDirectory);
};

#endif  // BITTORRENTCLIENT_TORRENTCLIENT_H
