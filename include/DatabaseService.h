#ifndef BITTORRENTCLIENT_DatabaseService_H
#define BITTORRENTCLIENT_DatabaseService_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <sqlite3.h>

#include <memory>
#include <string>
#include <tl/expected.hpp>

enum class DatabaseServiceError {
  OpenError,
  InsertError,
  QueryError,
  UpdateError
};

struct TorrentRecord {
  std::string id;  // hashinfo
  std::string name;
};

// SQLiteDeleter functor to properly close the SQLite connection
struct SQLiteDeleter {
  void operator()(sqlite3* db) const {
    if (db) {
      sqlite3_close(db);
    }
  }
};

class DatabaseService {
 private:
  std::shared_ptr<SQLite::Database> db;

 public:
  explicit DatabaseService(std::shared_ptr<SQLite::Database> dbState);

  virtual tl::expected<void, DatabaseServiceError> insertOne(
      std::string hashinfo, std::string name);

  virtual tl::expected<TorrentRecord, DatabaseServiceError> getTorrent(
      std::string hashinfo);

  virtual tl::expected<void, DatabaseServiceError> up();

  // deconsutrctor
  virtual ~DatabaseService();
};

std::shared_ptr<SQLite::Database> initDB(const std::string& dbName);

#endif  // BITTORRENTCLIENT_DatabaseService_H
