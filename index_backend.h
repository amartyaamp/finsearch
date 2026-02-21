#pragma once
#include "csv_parser.h"
#include <memory>
#include <string>
#include <vector>

using FaissSearchResults = std::pair<std::vector<float>, std::vector<long>>;

struct WindowMetadata {
  std::string start_date;
  std::string end_date;
};

struct SearchResult {
  float distance;
  long index;
  WindowMetadata metadata;
};

class IndexBackend {
public:
  virtual ~IndexBackend() = default;

  // Build an index from raw, continuous price data and associated metadata
  virtual void build_index(const std::vector<float> &raw_prices,
                           const std::vector<RowMetadata> &metadata,
                           int window_size) = 0;

  // Persist the current index to disk
  virtual void save_index(const std::string &index_path) = 0;

  // Load a persisted index from disk
  virtual void load_index(const std::string &index_path, int window_size) = 0;

  // Search the index with a raw query pattern
  virtual std::vector<SearchResult>
  search(const std::vector<float> &query_pattern, int k_neighbors,
         int window_size) = 0;
};
