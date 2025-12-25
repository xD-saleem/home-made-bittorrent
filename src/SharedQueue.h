
#ifndef BITTORRENTCLIENT_SHAREDQUEUE_H
#define BITTORRENTCLIENT_SHAREDQUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>

/**
 * Implementation of a thread-safe Queue. Code from
 * https://stackoverflow.com/questions/36762248/why-is-stdqueue-not-thread-safe
 */
template <typename T>
class SharedQueue {
 public:
  SharedQueue();
  ~SharedQueue();

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
SharedQueue<T>::SharedQueue() = default;

template <typename T>
SharedQueue<T>::~SharedQueue() = default;

template <typename T>
T SharedQueue<T>::front() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty(); });
  return queue_.front();
}

template <typename T>
T SharedQueue<T>::pop_front() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty(); });
  T front = std::move(queue_.front());
  queue_.pop_front();
  return front;
}

template <typename T>
void SharedQueue<T>::push_back(const T item) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(std::move(item));
  }
  cond_.notify_one();  // notify one waiting thread
}

template <typename T>
int SharedQueue<T>::size() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

template <typename T>
bool SharedQueue<T>::is_empty() {
  std::lock_guard<std::mutex> lock(mutex_);
  return size() == 0;
}

/**
 * Empties the queue
 */
template <typename T>
void SharedQueue<T>::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.empty();
  cond_.notify_one();
}

#endif  // BITTORRENTCLIENT_SHAREDQUEUE_H
