#include "connect.h"

#include <arpa/inet.h>  // for inet_pton
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

#include "utils.h"

constexpr int kReadTimeout = 300;
constexpr int kConnectTimeout = 5;

static bool setSocketBlocking(int sock, bool blocking) {
  if (sock < 0) {
    std::cerr << "Invalid socket descriptor\n";
    return false;
  }

  int flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) {
    std::cerr << "Failed to get socket flags\n";
    return false;
  }

  if (blocking)
    flags &= ~O_NONBLOCK;  // Clear O_NONBLOCK flag (blocking mode)
  else
    flags |= O_NONBLOCK;  // Set O_NONBLOCK flag (non-blocking mode)

  if (fcntl(sock, F_SETFL, flags) == -1) {
    std::cerr << "Failed to set socket flags\n";
    return false;
  }

  return true;
}

tl::expected<int, ConnectError> createConnection(const std::string& ip,
                                                 int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return tl::unexpected(ConnectError{"Socket creation error"});
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) <= 0) {
    close(sock);
    return tl::unexpected(ConnectError{"Invalid address: " + ip});
  }

  // Set socket to non-blocking mode
  if (!setSocketBlocking(sock, false)) {
    close(sock);
    return tl::unexpected(ConnectError{"Failed to set socket to NONBLOCK"});
  }

  connect(sock, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));

  fd_set fdset;
  timeval tv{.tv_sec = kConnectTimeout, .tv_usec = 0};
  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);

  if (select(sock + 1, nullptr, &fdset, nullptr, &tv) == 1) {
    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

    if (so_error == 0) {
      if (!setSocketBlocking(sock, true)) {
        close(sock);
        return tl::unexpected(ConnectError{"Failed to set socket to BLOCK"});
      }
      return sock;
    }
  }

  close(sock);
  return tl::unexpected(ConnectError{"Connection timeout to " + ip});
}

tl::expected<void, ConnectError> sendData(const int sock,
                                          const std::string& data) {
  int n = data.length();
  char buffer[n];
  for (int i = 0; i < n; i++) {
    buffer[i] = data[i];
  }

  int res = send(sock, buffer, n, 0);
  if (res < 0) {
    return tl::unexpected(
        ConnectError{"Failed to send data to socket " + std::to_string(sock)});
  }

  return {};
}

tl::expected<std::string, ConnectError> receiveData(int sock,
                                                    uint32_t bufferSize) {
  std::string reply;
  constexpr int kLengthIndicatorSize = 4;

  if (bufferSize == 0) {
    struct pollfd fd{sock, POLLIN, 0};
    int ret = poll(&fd, 1, kReadTimeout);

    if (ret < 0) {
      return tl::unexpected(
          ConnectError{"Polling error on socket" + std::to_string(sock)});
    }
    if (ret == 0) {
      return tl::unexpected(
          ConnectError{"Read timeout on socket " + std::to_string(sock)});
    }

    char length_buffer[kLengthIndicatorSize] = {};
    if (recv(sock, length_buffer, kLengthIndicatorSize, 0) !=
        kLengthIndicatorSize) {
      return tl::unexpected(ConnectError{
          "Failed to read message length from socket " + std::to_string(sock)});
    }

    bufferSize = bytesToInt(std::string(length_buffer, kLengthIndicatorSize));
  }

  if (bufferSize > std::numeric_limits<uint16_t>::max()) {
    return tl::unexpected(
        ConnectError{"Buffer size too large: " + std::to_string(bufferSize)});
  }

  std::vector<char> buffer(bufferSize);
  size_t bytes_read = 0;
  auto start_time = std::chrono::steady_clock::now();

  while (bytes_read < bufferSize) {
    if (std::chrono::steady_clock::now() - start_time >
        std::chrono::milliseconds(kReadTimeout)) {
      return tl::unexpected(
          ConnectError{"Read timeout on socket " + std::to_string(sock)});
    }

    int64_t result =
        recv(sock, buffer.data() + bytes_read, bufferSize - bytes_read, 0);
    if (result <= 0) {
      return tl::unexpected(ConnectError{"Failed to read data from socket " +
                                         std::to_string(sock)});
    }
    bytes_read += result;
  }

  return std::string(buffer.begin(), buffer.end());
}
