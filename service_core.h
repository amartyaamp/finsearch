#pragma once
#include "faiss_index_backend.h"
#include <shared_mutex>
#include <memory>
#include <string>
#include <vector>

class FinSearchService {
private:
    std::unique_ptr<FaissIndexBackend> backend;
    mutable std::shared_mutex rw_mutex;

public:
    FinSearchService();
    ~FinSearchService();

    // Prevent copies
    FinSearchService(const FinSearchService&) = delete;
    FinSearchService& operator=(const FinSearchService&) = delete;

    void build_index(const std::vector<std::vector<float>> &feature_series,
                     const std::vector<RowMetadata> &metadata,
                     int window_size);

    void save_index(const std::string &index_path);

    void load_index(const std::string &index_path, int window_size);

    std::vector<SearchResult>
    search(const std::vector<std::vector<float>> &query_series, int k_neighbors,
           int window_size);

    size_t get_total_vectors() const;
};
