
#ifndef LOGGER_H
#define LOGGER_H

#include <fmt/core.h>

#include <string>

class Logger {
 public:
  using LogFunction =
      void (*)(const std::string &);  // Function pointer for logging

  explicit Logger(LogFunction logFunc = defaultLogFunc);  // Constructor with DI
  static void log(const std::string &message);

 private:
  static LogFunction logFunction;
  static void defaultLogFunc(const std::string &message);
};

#endif  // LOGGER_H
