#pragma once
#include <string>
#include <vector>

// Extensible struct for row-level metadata (like timestamps)
struct RowMetadata {
  std::string timestamp;
  // Extensible for future features
};

// Extensible struct for a complete parsed time series dataset
struct TimeSeriesData {
  std::vector<RowMetadata> row_metadata;
  std::vector<float> prices;
  // Extensible for multivariate arrays
};

// Function to load timestamps and Close prices from a generic financial CSV
// file.
TimeSeriesData load_csv_data(const std::string &filename);

// Legacy function to load just the Close prices from a simple financial CSV
// file. Maintained to avoid breaking existing code that only needs floats.
std::vector<float> load_csv_close_prices(const std::string &filename);

// Function to load query patterns from a CSV file.
// Each row represents a separate query pattern.
std::vector<std::vector<float>>
load_query_patterns(const std::string &filename);