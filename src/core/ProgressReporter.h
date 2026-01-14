
#ifndef BITTORRENTCLIENT_PROGRESSREPORTER_H
#define BITTORRENTCLIENT_PROGRESSREPORTER_H

#include <mutex>

class ProgressReporter {
 private:
  // Deps
  std::mutex lock_;

 public:
  explicit ProgressReporter();
  // Destructor
  ~ProgressReporter() = default;

  void start();

  void trackProgress();

  void displayProgressBar();
};

#endif  // BITTORRENTCLIENT_PROGRESSREPORTER_H
