#ifndef BITTORRENTCLIENT_SHAREDQUEUE_H
#define BITTORRENTCLIENT_SHAREDQUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

template <typename T>
class SharedQueue {
 public:
  SharedQueue() = default;
  ~SharedQueue() = default;

  // Non-copyable
  SharedQueue(const SharedQueue&) = delete;
  SharedQueue& operator=(const SharedQueue&) = delete;

  // Movable
  SharedQueue(SharedQueue&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.mutex_);
    queue_ = std::move(other.queue_);
  }
  SharedQueue& operator=(SharedQueue&& other) noexcept {
    if (this != &other) {
      std::lock_guard<std::mutex> lock1(mutex_);
      std::lock_guard<std::mutex> lock2(other.mutex_);
      queue_ = std::move(other.queue_);
    }
    return *this;
  }

  // Add an element (lvalue)
  void push_back(const T& item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push_back(item);
    }
    cond_.notify_one();
  }

  // Add an element (rvalue / move)
  void push_back(T&& item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push_back(std::move(item));
    }
    cond_.notify_one();
  }

  // Remove and return the front element (blocking)
  T pop_front() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !queue_.empty(); });
    T front = std::move(queue_.front());
    queue_.pop_front();
    return front;
  }

  // Peek at the front element (blocking)
  T front() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !queue_.empty(); });
    return queue_.front();  // copy or reference depending on T
  }

  // Clear the queue
  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
    cond_.notify_all();  // notify anyone waiting
  }

  // Size of the queue
  int size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(queue_.size());
  }

  // Is the queue empty?
  bool empty() const { return size() == 0; }

 private:
  mutable std::mutex mutex_;
  std::condition_variable cond_;
  std::deque<T> queue_;
};
#endif  // BITTORRENTCLIENT_SHAREDQUEUE_H
