
#include "core/TorrentState.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <infra/DatabaseService.h>
#include <infra/DatabaseService_mock.h>

TEST(TorrentState, storeStateHandlesInsertError) {
  // Create a mock database service
  std::shared_ptr<MockDatabaseService> db_service =
      std::make_shared<MockDatabaseService>();

  TorrentState ts = TorrentState(db_service);

  EXPECT_CALL(*db_service, insertOne("mockHashMock", "mockName"))
      .WillOnce(::testing::Return(tl::unexpected<DatabaseServiceError>{
          DatabaseServiceError::kInsertError}));

  auto val = ts.storeState("mockHashMock", "mockName");

  ASSERT_FALSE(val.has_value())
      << "Expected storeState to return an error, but it succeeded.";

  EXPECT_EQ(val.error().message, "failed to save hashinfo");
}

TEST(TorrentState, storeStateHandlesInsertOK) {
  std::shared_ptr<MockDatabaseService> db_service =
      std::make_shared<MockDatabaseService>();

  TorrentState ts = TorrentState(db_service);

  EXPECT_CALL(*db_service, insertOne("mockHashMock1", "mockName1"))
      .WillOnce(::testing::Return(tl::expected<void, DatabaseServiceError>{}));

  auto val = ts.storeState("mockHashMock1", "mockName1");

  ASSERT_TRUE(val.has_value());
}
