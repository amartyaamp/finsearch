#include "csv_data_source.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

CsvDataSource::CsvDataSource(const std::string &filepath,
                             const DataSourceConfig &config)
    : filepath_(filepath), config_(config) {}

TimeSeriesData CsvDataSource::load() {
  TimeSeriesData data;
  data.feature_names = config_.feature_names;

  std::ifstream file(filepath_);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file: " << filepath_ << std::endl;
    return data;
  }

  // Pre-size the features matrix: one inner vector per configured feature.
  // Done after the file-open check so a missing file returns an empty struct.
  data.features.resize(config_.feature_col_indices.size());

  // Skip the header line.
  std::string line;
  std::getline(file, line);

  // Determine the highest column index we need to access so we can validate
  // each row before touching it.
  int max_col_needed = config_.timestamp_col_index;
  for (int idx : config_.feature_col_indices) {
    if (idx > max_col_needed)
      max_col_needed = idx;
  }

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string cell;
    std::vector<std::string> row;

    while (std::getline(ss, cell, ',')) {
      row.push_back(cell);
    }

    // Skip rows that don't have enough columns for all required indices.
    if ((int)row.size() <= max_col_needed)
      continue;

    // Parse every configured feature column; if any one fails, skip the row.
    bool row_ok = true;
    std::vector<float> parsed(config_.feature_col_indices.size());
    for (size_t fi = 0; fi < config_.feature_col_indices.size(); ++fi) {
      try {
        parsed[fi] = std::stof(row[config_.feature_col_indices[fi]]);
      } catch (...) {
        std::cerr << "Warning: Failed to parse feature at line: " << line
                  << std::endl;
        row_ok = false;
        break;
      }
    }
    if (!row_ok)
      continue;

    // Commit parsed features.
    for (size_t fi = 0; fi < parsed.size(); ++fi) {
      data.features[fi].push_back(parsed[fi]);
    }

    // Populate legacy fields for backward compatibility.
    // features[0] → prices, features[1] → volumes (if they exist).
    if (parsed.size() >= 1)
      data.prices.push_back(parsed[0]);
    if (parsed.size() >= 2)
      data.volumes.push_back(parsed[1]);

    // Store timestamp.
    RowMetadata meta;
    meta.timestamp = row[config_.timestamp_col_index];
    data.row_metadata.push_back(meta);
  }

  return data;
}

DataSourceConfig default_ohlcv_config() {
  DataSourceConfig cfg;
  cfg.timestamp_col_index = 0;      // "date" column
  cfg.feature_col_indices = {4, 5}; // "close", "volume"
  cfg.feature_names = {"close", "volume"};
  return cfg;
}
