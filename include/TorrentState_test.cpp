
#include "TorrentState.h"

#include <DatabaseService.h>
#include <DatabaseService_mock.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// TEST(TorrentState, storeState) {
//   MockDatabaseService dbService;
//
//   TorrentState ts = TorrentState(&dbService);
//
//   EXPECT_CALL(dbService, insertOne("mockHashMock", "mockName"))
//       .WillOnce(::testing::Return(tl::unexpected<DatabaseServiceError>{
//           DatabaseServiceError::InsertError}));
//
//   auto val = ts.storeState("mockHashMock", "mockName");
//
//   // ASSERT_FALSE(val.has_value());  // Check if it returns an error
//   // EXPECT_EQ(val.error(), DatabaseServiceError::InsertError);
// }
//
