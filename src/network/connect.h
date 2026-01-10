
#ifndef BITTORRENTCLIENT_CONNECT_H
#define BITTORRENTCLIENT_CONNECT_H
#include <cstdint>
#include <string>
#include <tl/expected.hpp>

struct ConnectError {
  std::string message;
};

// Networks
tl::expected<int, ConnectError> createConnection(const std::string& ip,
                                                 int port);
tl::expected<void, ConnectError> sendData(int sock, const std::string& data);
tl::expected<std::string, ConnectError> receiveData(int sock,
                                                    uint32_t bufferSize = 0);

#endif  // BITTORRENTCLIENT_CONNECT_H
