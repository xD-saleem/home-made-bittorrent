#include "bencode_parser.h"

#include <gtest/gtest.h>

// Test fixture for BencodeParser
class BencodeParserTest : public ::testing::Test {
 protected:
  BencodeParser parser;
};

// Test case for the parse method
TEST_F(BencodeParserTest, ParseTest) {
  // Test input
  std::string bencoded_string = "example_bencoded_string";

  // Expected output
  int expected_result = 0;

  // Call the parse method
  int actual_result = parser.parse(bencoded_string, 0);

  // Check if the actual result matches the expected result
  ASSERT_EQ(expected_result, actual_result);
}

int main(int argc, char** argv) {
  // Initialize Google Test
  ::testing::InitGoogleTest(&argc, argv);

  // Run tests
  return RUN_ALL_TESTS();
}

#include <fmt/core.h>

#include <string>

// Constructor
BencodeParser::BencodeParser() {}

// Destructor
BencodeParser::~BencodeParser() {}

// Parse a bencoded string

int BencodeParser::parse(std::string bencoded_string, int index) {
  fmt::print("Parsing bencoded string: {}\n", bencoded_string);

  return 0;
}

