
#include "Logger.h"

// Initialize static log function with default
Logger::LogFunction Logger::log_function_ = Logger::defaultLogFunc;

// Default log function using fmt
void Logger::defaultLogFunc(const std::string& message) {
  fmt::print("[LOG]: {}\n", message);
}

// Constructor allows function injection
Logger::Logger(LogFunction logFunc) {
  log_function_ = logFunc;
  log_function_("Logger initialized!");
}

// Static log method
void Logger::log(const std::string& message) { log_function_(message); }
