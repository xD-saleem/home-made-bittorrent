#include "DatabaseService.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <memory>
#include <string>
#include <tl/expected.hpp>

// SQL
// CREATE TABLE Torrents (
// 	name TEXT NOT NULL,
// 	hashinfo TEXT NOT NULL,
// 	CONSTRAINT Torrents_PK PRIMARY KEY (hashinfo)

const std::string DB_STATE = std::string("torrent_state.db");

// Destructor can remain default
DatabaseService::~DatabaseService() = default;

// Function to initialize the SQLite database
std::shared_ptr<SQLite::Database> initDB(const std::string &dbName) {
  auto dbConn = std::make_shared<SQLite::Database>(
      dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  return dbConn;
}

// Constructor implementation
DatabaseService::DatabaseService(std::shared_ptr<SQLite::Database> dbState,
                                 std::shared_ptr<Logger> logger) {
  db = dbState;
  if (!db) {
    logger->log("Failed to initialize the database");
  } else {
    logger->log("Database initialized successfully");
  }
}

tl::expected<void, DatabaseServiceError> DatabaseService::up() {
  SQLite::Statement query{
      *this->db,
      "CREATE TABLE Torrents(name TEXT NOT NULL, hashinfo TEXT NOT NULL, "
      "CONSTRAINT Torrents_PK PRIMARY KEY(hashinfo));"};

  int d = query.exec();
  logger->log(fmt::format("applied migration: {}", d));
  return {};
}

tl::expected<void, DatabaseServiceError> DatabaseService::insertOne(
    std::string hashinfo, std::string name) {
  SQLite::Statement query{
      *this->db, "INSERT INTO Torrents (name, hashinfo) VALUES (?, ?)"};

  query.bind(1, name);
  query.bind(2, hashinfo);

  int d = query.exec();
  logger->log(fmt::format("inserted {}", d));

  return {};
}

tl::expected<TorrentRecord, DatabaseServiceError> DatabaseService::getTorrent(
    std::string hashinfo) {
  SQLite::Statement query{*this->db,
                          "SELECT * FROM Torrents WHERE hashinfo = ?"};

  query.bind(1, hashinfo);

  int d = query.executeStep();
  if (d) {
    TorrentRecord t;
    t.name = query.getColumn(0).getString();
    t.id = query.getColumn(1).getString();
    return t;
  }

  return {};
}
