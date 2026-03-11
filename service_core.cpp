#include "service_core.h"
#include <stdexcept>
#include <shared_mutex>
#include <mutex>

FinSearchService::FinSearchService() {
    backend = std::make_unique<FaissIndexBackend>();
}

FinSearchService::~FinSearchService() = default;

void FinSearchService::build_index(const std::vector<std::vector<float>> &feature_series,
                                   const std::vector<RowMetadata> &metadata,
                                   int window_size) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    backend->build_index(feature_series, metadata, window_size);
}

void FinSearchService::save_index(const std::string &index_path) {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    backend->save_index(index_path);
}

void FinSearchService::load_index(const std::string &index_path, int window_size) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    backend->load_index(index_path, window_size);
}

std::vector<SearchResult>
FinSearchService::search(const std::vector<std::vector<float>> &query_series, int k_neighbors,
                         int window_size) {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    return backend->search(query_series, k_neighbors, window_size);
}

size_t FinSearchService::get_total_vectors() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    return backend->get_total_vectors();
}
