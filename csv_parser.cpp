#include "csv_parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const int DATE_INDEX = 0;        // 0-based index for the 'date' column
const int CLOSE_PRICE_INDEX = 4; // 0-based index for the 'close' column

TimeSeriesData load_csv_data(const std::string &filename) {
  TimeSeriesData data;
  std::ifstream file(filename);
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file: " << filename << std::endl;
    return data;
  }

  // Skip header line
  std::getline(file, line);

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string cell;
    std::vector<std::string> row;

    // Split by comma
    while (std::getline(ss, cell, ',')) {
      row.push_back(cell);
    }

    // Check if the row has enough columns (at least 5 for index 4)
    if (row.size() > CLOSE_PRICE_INDEX) {
      try {
        // Parse the close price (index 4)
        float price = std::stof(row[CLOSE_PRICE_INDEX]);
        data.prices.push_back(price);

        // Store the timestamp (index 0)
        RowMetadata meta;
        meta.timestamp = row[DATE_INDEX];
        data.row_metadata.push_back(meta);
      } catch (...) {
        // Skip lines with unreadable price data
        std::cerr << "Warning: Failed to parse price at line: " << line
                  << std::endl;
        continue;
      }
    }
  }
  return data;
}

std::vector<float> load_csv_close_prices(const std::string &filename) {
  TimeSeriesData data = load_csv_data(filename);
  return data.prices;
}