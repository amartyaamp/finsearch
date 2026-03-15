#pragma once
#include "csv_parser.h"
#include <string>
#include <vector>

// Configuration that tells a DataSource which columns to read and how to
// label them. This is the heart of the "arbitrary data sources" abstraction —
// callers can point to any CSV layout without touching engine code.
struct DataSourceConfig {
  int timestamp_col_index = 0; // column index for the row timestamp/date

  // Ordered list of column indices to treat as numeric feature series.
  // features[i] in TimeSeriesData corresponds to feature_col_indices[i].
  std::vector<int> feature_col_indices;

  // Human-readable label for each feature (parallel to feature_col_indices).
  std::vector<std::string> feature_names;
};

// Returns the default OHLCV config that matches the legacy hard-coded
// behaviour: timestamp = col 0 ("date"), features = [col 4 ("close"), col 5
// ("volume")].
DataSourceConfig default_ohlcv_config();

// Pure-virtual interface every concrete data source must implement.
// load() returns a fully populated TimeSeriesData including:
//   - row_metadata  (timestamps)
//   - features      (generic matrix: features[i] = time-series for feature i)
//   - feature_names (labels)
//   - prices        (legacy alias for features[0], if available)
//   - volumes       (legacy alias for features[1], if available)
class DataSource {
public:
  virtual ~DataSource() = default;
  virtual TimeSeriesData load() = 0;
};
