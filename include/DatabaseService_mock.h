
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "DatabaseService.h"

class MockDatabaseService : public DatabaseService {
 public:
  MockDatabaseService() : DatabaseService(nullptr) {}

  MOCK_METHOD((tl::expected<void, DatabaseServiceError>), up, (), (override));

  MOCK_METHOD((tl::expected<void, DatabaseServiceError>), insertOne,
              (std::string, std::string), (override));

  MOCK_METHOD((tl::expected<TorrentRecord, DatabaseServiceError>), getTorrent,
              (std::string), (override));
};

