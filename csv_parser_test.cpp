#include "csv_parser.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

// Define a temporary test file name
const std::string TEST_CSV_PATH = "test_data_parser.csv";

// Helper function to create the temporary CSV file
void create_test_csv() {
  std::ofstream file(TEST_CSV_PATH);
  file << "date,open,high,low,close,volume\n";
  file << "2023-01-01,100,105,95,102.50,1000\n"; // Index 4: 102.50
  file << "2023-01-02,102,107,97,104.00,1200\n"; // Index 4: 104.00
  file << "Invalid Line\n";                      // Should be skipped
  file << "2023-01-03,104,109,99,106.50,1500\n"; // Index 4: 106.50
  file.close();
}

// Define the Test Suite
TEST(CsvParserTest, ParsesClosePricesCorrectly) {
  create_test_csv(); // Create the temporary file

  std::vector<float> prices = load_csv_close_prices(TEST_CSV_PATH);

  // Expected prices based on the temporary CSV
  std::vector<float> expected = {102.50f, 104.00f, 106.50f};

  // 1. Check the count
  ASSERT_EQ(prices.size(), expected.size())
      << "Should have parsed 3 valid rows.";

  // 2. Check the values
  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_NEAR(prices[i], expected[i], 1e-4)
        << "Price mismatch at index " << i;
  }

  // Clean up the temporary file
  fs::remove(TEST_CSV_PATH);
}

// Test case for a non-existent file
TEST(CsvParserTest, HandlesMissingFile) {
  std::vector<float> prices = load_csv_close_prices("non_existent_file.csv");
  ASSERT_TRUE(prices.empty());
}