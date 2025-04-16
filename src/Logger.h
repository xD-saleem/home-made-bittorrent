
#ifndef LOGGER_H
#define LOGGER_H

#include <fmt/color.h>
#include <fmt/core.h>

#include <string>

class Logger {
 public:
  using LogFunction =
      void (*)(const std::string &);  // Function pointer for logging

  explicit Logger(LogFunction logFunc = defaultLogFunc);  // Constructor with DI
  static void log(const std::string &message);

  static void custom_log_function(const std::string &message) {
    fmt::print(fg(fmt::color::cyan), "[LOG]: {}\n", message);
  }

 private:
  static LogFunction log_function_;
  static void defaultLogFunc(const std::string &message);
};

#endif  // LOGGER_H
