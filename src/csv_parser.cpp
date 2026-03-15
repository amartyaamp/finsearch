#include "csv_parser.h"
#include "csv_data_source.h"
#include <string>
#include <vector>

TimeSeriesData load_csv_data(const std::string &filename) {
  // Delegate to CsvDataSource with the default OHLCV column layout.
  // This preserves the exact behaviour of the previous hard-coded
  // implementation.
  CsvDataSource source(filename, default_ohlcv_config());
  return source.load();
}

std::vector<float> load_csv_close_prices(const std::string &filename) {
  TimeSeriesData data = load_csv_data(filename);
  return data.prices;
}