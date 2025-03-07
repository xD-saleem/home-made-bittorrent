
#include "TorrentState.h"

#include <DatabaseService.h>
#include <DatabaseService_mock.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(TorrentState, storeStateHandlesInsertError) {
  // Create a mock database service
  std::shared_ptr<MockDatabaseService> dbService =
      std::make_shared<MockDatabaseService>();

  TorrentState ts = TorrentState(dbService);

  EXPECT_CALL(*dbService, insertOne("mockHashMock", "mockName"))
      .WillOnce(::testing::Return(tl::unexpected<DatabaseServiceError>{
          DatabaseServiceError::InsertError}));

  auto val = ts.storeState("mockHashMock", "mockName");

  ASSERT_FALSE(val.has_value())
      << "Expected storeState to return an error, but it succeeded.";

  EXPECT_EQ(val.error().message, "failed to save hashinfo");
}

TEST(TorrentState, storeStateHandlesInsertOK) {
  std::shared_ptr<MockDatabaseService> dbService =
      std::make_shared<MockDatabaseService>();

  TorrentState ts = TorrentState(dbService);

  EXPECT_CALL(*dbService, insertOne("mockHashMock1", "mockName1"))
      .WillOnce(::testing::Return(tl::expected<void, DatabaseServiceError>{}));

  auto val = ts.storeState("mockHashMock1", "mockName1");

  ASSERT_TRUE(val.has_value());
}
