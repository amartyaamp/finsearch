#pragma once

#include "index_backend.h"
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>

class FaissIndexBackend : public IndexBackend {
private:
  std::unique_ptr<faiss::IndexFlatL2> index;
  std::vector<WindowMetadata> metadata_store;

public:
  FaissIndexBackend() : index(nullptr) {}
  ~FaissIndexBackend() override = default;

  void build_index(const std::vector<float> &raw_prices,
                   const std::vector<RowMetadata> &metadata,
                   int window_size) override;
  void save_index(const std::string &index_path) override;
  void load_index(const std::string &index_path, int window_size) override;
  std::vector<SearchResult> search(const std::vector<float> &query_pattern,
                                   int k_neighbors, int window_size) override;

  // Exposed for testing purposes
  size_t get_total_vectors() const { return index ? index->ntotal : 0; }
  size_t get_total_metadata() const { return metadata_store.size(); }
};
