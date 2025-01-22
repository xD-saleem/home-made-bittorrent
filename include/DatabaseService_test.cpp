
#include "DatabaseService.h"

#include <fmt/core.h>
#include <gtest/gtest.h>

#include <cstdlib>

TEST(DatabaseService, InsertOneAndGetOne) {
  std::string memoryDb = ":memory:";
  auto db = initDB(memoryDb);

  DatabaseService databaseSvc = DatabaseService(db);

  databaseSvc.up();

  std::string mockHash = "mockHash";
  std::string mockTorrentName = "mockTorrentName";

  tl::expected<void, DatabaseServiceError> res =
      databaseSvc.insertOne(mockHash, mockTorrentName);

  EXPECT_TRUE(res.has_value());

  tl::expected<TorrentRecord, DatabaseServiceError> obj =
      databaseSvc.getTorrent(mockHash);

  EXPECT_EQ(obj->id, mockHash);
  EXPECT_EQ(obj->name, mockTorrentName);
}

