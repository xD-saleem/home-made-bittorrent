
#include "Logger.h"

// Initialize static log function with default
Logger::LogFunction Logger::logFunction = Logger::defaultLogFunc;

// Default log function using fmt
void Logger::defaultLogFunc(const std::string &message) {
  fmt::print("[LOG]: {}\n", message);
}

// Constructor allows function injection
Logger::Logger(LogFunction logFunc) {
  logFunction = logFunc;
  logFunction("Logger initialized!");
}

// Static log method
void Logger::log(const std::string &message) { logFunction(message); }
