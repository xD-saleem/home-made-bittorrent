
#include "DatabaseService.h"

#include <fmt/core.h>
#include <gtest/gtest.h>

#include <cstdlib>

static std::shared_ptr<Logger> logger =
    std::make_shared<Logger>(Logger::custom_log_function);

TEST(DatabaseService, InsertOneAndGetOne) {
  std::string memory_db = ":memory:";

  auto db = initDB(memory_db);
  DatabaseService database_svc = DatabaseService(db, logger);

  database_svc.up();

  std::string mock_hash = "mockHash";
  std::string mock_torrent_name = "mockTorrentName";

  tl::expected<void, DatabaseServiceError> res =
      database_svc.insertOne(mock_hash, mock_torrent_name);

  EXPECT_TRUE(res.has_value());

  tl::expected<TorrentRecord, DatabaseServiceError> obj =
      database_svc.getTorrent(mock_hash);

  EXPECT_EQ(obj->id, mock_hash);
  EXPECT_EQ(obj->name, mock_torrent_name);
}
