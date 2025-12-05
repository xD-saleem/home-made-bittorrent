#ifndef BITTORRENTCLIENT_SHAREDQUEUE_H
#define BITTORRENTCLIENT_SHAREDQUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>

template <typename T>
class SharedQueue {
 public:
  SharedQueue() = default;
  ~SharedQueue() = default;

  // non-copyable
  SharedQueue(const SharedQueue&) = delete;
  SharedQueue& operator=(const SharedQueue&) = delete;

  // move-constructible
  SharedQueue(SharedQueue&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.mutex_);
    queue_ = std::move(other.queue_);
    // mutex_ and cond_ remain default-constructed for the new object
  }

  // move-assignable
  SharedQueue& operator=(SharedQueue&& other) noexcept {
    if (this != &other) {
      std::lock(mutex_, other.mutex_);
      std::lock_guard<std::mutex> lhs_lock(mutex_, std::adopt_lock);
      std::lock_guard<std::mutex> rhs_lock(other.mutex_, std::adopt_lock);
      queue_ = std::move(other.queue_);
    }
    return *this;
  }

  // Blocking front()
  T& front() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !queue_.empty(); });
    return queue_.front();
  }

  // Blocking pop_front()
  T pop_front() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !queue_.empty(); });
    T item = std::move(queue_.front());
    queue_.pop_front();
    return item;
  }

  // Push by copy
  void push_back(const T& item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push_back(item);
    }
    cond_.notify_one();
  }

  // Push by move
  void push_back(T&& item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push_back(std::move(item));
    }
    cond_.notify_one();
  }

  void clear() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::deque<T>().swap(queue_);
    }
    cond_.notify_all();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  int size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(queue_.size());
  }

 private:
  std::deque<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cond_;
};

#endif
