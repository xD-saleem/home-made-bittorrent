#ifndef BITTORRENTCLIENT_QUEUE_H
#define BITTORRENTCLIENT_QUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <sstream>
#include <thread>

#include "Logger.h"

inline std::string thread_id_str() {
  std::ostringstream oss;
  oss << std::this_thread::get_id();
  return oss.str();
}

template <typename T>
class Queue {
 public:
  Queue() = default;
  ~Queue() = default;

  T front();
  T pop_front();

  void push_back(T item);
  void clear();

  int size();
  bool is_empty();

 private:
  std::deque<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

template <typename T>
T Queue<T>::front() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty(); });

  return queue_.front();
}

template <typename T>
T Queue<T>::pop_front() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty(); });

  T front = std::move(queue_.front());
  queue_.pop_front();

  return front;
}

template <typename T>
void Queue<T>::push_back(T item) {
  {
    std::lock_guard<std::mutex> lock(mutex_);

    queue_.push_back(std::move(item));
  }

  cond_.notify_one();
}

template <typename T>
int Queue<T>::size() {
  std::lock_guard<std::mutex> lock(mutex_);
  return static_cast<int>(queue_.size());
}

template <typename T>
bool Queue<T>::is_empty() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();  // avoid double locking via size()
}

template <typename T>
void Queue<T>::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::deque<T> empty;

  queue_.swap(empty);

  cond_.notify_all();
}

#endif  // BITTORRENTCLIENT_QUEUE_H
