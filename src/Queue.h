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
  // Logger::log(
  // fmt::format("[Queue::front] tid={} waiting for item", thread_id_str()));

  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty(); });

  // Logger::log(fmt::format("[Queue::front] tid={} woke up, size={}",
  //                         thread_id_str(), queue_.size()));

  return queue_.front();
}

template <typename T>
T Queue<T>::pop_front() {
  // Logger::log(fmt::format("[Queue::pop_front] tid={} waiting for item",
  // thread_id_str()));

  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return !queue_.empty(); });

  // Logger::log(
  // fmt::format("[Queue::pop_front] tid={} popping item, size_before={}",
  // thread_id_str(), queue_.size()));

  T front = std::move(queue_.front());
  queue_.pop_front();

  // Logger::log(
  //     fmt::format("[Queue::pop_front] tid={} item popped, size_after={}",
  //                 thread_id_str(), queue_.size()));

  return front;
}

template <typename T>
void Queue<T>::push_back(T item) {
  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Logger::log(
    //     fmt::format("[Queue::push_back] tid={} pushing item, size_before={}",
    //                 thread_id_str(), queue_.size()));

    queue_.push_back(std::move(item));

    // Logger::log(
    //     fmt::format("[Queue::push_back] tid={} pushed item, size_after={}",
    //                 thread_id_str(), queue_.size()));
  }

  // Logger::log(fmt::format("[Queue::push_back] tid={} notifying one waiter",
  // thread_id_str()));

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

/**
 * Empties the queue
 */
template <typename T>
void Queue<T>::clear() {
  std::lock_guard<std::mutex> lock(mutex_);

  // Logger::log(
  //     fmt::format("[Queue::clear] tid={} clearing queue, size_before={}",
  //                 thread_id_str(), queue_.size()));

  std::deque<T> empty;
  queue_.swap(empty);

  // Logger::log(fmt::format("[Queue::clear] tid={} queue cleared, notifying
  // all",
  //                         thread_id_str()));

  cond_.notify_all();
}

#endif  // BITTORRENTCLIENT_QUEUE_H
