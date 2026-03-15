#include "csv_data_source.h"
#include "csv_parser.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ─── Helpers ────────────────────────────────────────────────────────────────

const std::string TEST_CSV = "test_data_source.csv";

// Creates a small OHLCV CSV that matches the legacy format:
//   date, open, high, low, close, volume
void create_ohlcv_csv() {
  std::ofstream f(TEST_CSV);
  f << "date,open,high,low,close,volume\n";
  f << "2023-01-01,100,110,90,102.5,1000\n";
  f << "2023-01-02,102,112,92,104.0,1200\n";
  f << "Invalid Line\n"; // should be skipped
  f << "2023-01-03,104,114,94,106.5,1500\n";
  f.close();
}

void remove_test_csv() {
  if (fs::exists(TEST_CSV))
    fs::remove(TEST_CSV);
}

// ─── Tests ──────────────────────────────────────────────────────────────────

// 1. Default OHLCV config should produce identical data to load_csv_data().
TEST(CsvDataSourceTest, DefaultConfigMatchesLegacy) {
  create_ohlcv_csv();

  // Legacy loader
  TimeSeriesData legacy = load_csv_data(TEST_CSV);

  // New CsvDataSource with default config
  CsvDataSource source(TEST_CSV, default_ohlcv_config());
  TimeSeriesData modern = source.load();

  // Prices and volumes must be identical
  ASSERT_EQ(modern.prices.size(), legacy.prices.size());
  ASSERT_EQ(modern.volumes.size(), legacy.volumes.size());
  for (size_t i = 0; i < legacy.prices.size(); ++i) {
    EXPECT_NEAR(modern.prices[i], legacy.prices[i], 1e-4f);
    EXPECT_NEAR(modern.volumes[i], legacy.volumes[i], 1e-4f);
  }

  // Timestamps must match
  ASSERT_EQ(modern.row_metadata.size(), legacy.row_metadata.size());
  for (size_t i = 0; i < legacy.row_metadata.size(); ++i) {
    EXPECT_EQ(modern.row_metadata[i].timestamp,
              legacy.row_metadata[i].timestamp);
  }

  remove_test_csv();
}

// 2. Custom config: load open (col 1) and high (col 2) instead of close/volume.
TEST(CsvDataSourceTest, CustomColumnConfig) {
  create_ohlcv_csv();

  DataSourceConfig cfg;
  cfg.timestamp_col_index = 0;
  cfg.feature_col_indices = {1, 2}; // open, high
  cfg.feature_names = {"open", "high"};

  CsvDataSource source(TEST_CSV, cfg);
  TimeSeriesData data = source.load();

  // 3 valid rows → 3 entries per feature series
  ASSERT_EQ(data.features.size(), 2u);
  ASSERT_EQ(data.features[0].size(), 3u); // open values
  ASSERT_EQ(data.features[1].size(), 3u); // high values

  // Check open values: {100, 102, 104}
  EXPECT_NEAR(data.features[0][0], 100.0f, 1e-4f);
  EXPECT_NEAR(data.features[0][1], 102.0f, 1e-4f);
  EXPECT_NEAR(data.features[0][2], 104.0f, 1e-4f);

  // Check high values: {110, 112, 114}
  EXPECT_NEAR(data.features[1][0], 110.0f, 1e-4f);
  EXPECT_NEAR(data.features[1][1], 112.0f, 1e-4f);
  EXPECT_NEAR(data.features[1][2], 114.0f, 1e-4f);

  remove_test_csv();
}

// 3. feature_names in the config should be surfaced in TimeSeriesData.
TEST(CsvDataSourceTest, FeatureNamesAreSet) {
  create_ohlcv_csv();

  CsvDataSource source(TEST_CSV, default_ohlcv_config());
  TimeSeriesData data = source.load();

  ASSERT_EQ(data.feature_names.size(), 2u);
  EXPECT_EQ(data.feature_names[0], "close");
  EXPECT_EQ(data.feature_names[1], "volume");

  remove_test_csv();
}

// 4. Missing file should return empty TimeSeriesData without crashing.
TEST(CsvDataSourceTest, MissingFileHandledGracefully) {
  CsvDataSource source("no_such_file.csv", default_ohlcv_config());
  TimeSeriesData data = source.load();

  EXPECT_TRUE(data.prices.empty());
  EXPECT_TRUE(data.volumes.empty());
  EXPECT_TRUE(data.features.empty());
  EXPECT_TRUE(data.row_metadata.empty());
}
