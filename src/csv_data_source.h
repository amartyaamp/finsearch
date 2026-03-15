#pragma once
#include "data_source.h"
#include <string>

// CSV implementation of DataSource.
// Reads any CSV file and extracts columns as configured by DataSourceConfig.
// By default (using default_ohlcv_config()) it behaves identically to the
// legacy load_csv_data() function, ensuring full backward compatibility.
class CsvDataSource : public DataSource {
public:
  // Construct with a file path and a column-mapping config.
  CsvDataSource(const std::string &filepath, const DataSourceConfig &config);

  // Load and return all time-series data from the CSV.
  TimeSeriesData load() override;

private:
  std::string filepath_;
  DataSourceConfig config_;
};
