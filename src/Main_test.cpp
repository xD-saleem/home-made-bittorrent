#include <gtest/gtest.h>

int main(int argc, char **argv) {
  // Initialize Google Test framework
  ::testing::InitGoogleTest(&argc, argv);
  // Run tests
  return RUN_ALL_TESTS();
}
