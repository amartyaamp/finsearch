#include "faiss_index_backend.h"
#include "engine.h"
#include <iostream>

void FaissIndexBackend::build_index(const std::vector<float> &raw_prices,
                                    int window_size) {
  const int D = window_size;

  // 1. Preprocessing: Extract and Normalize all index patterns
  auto patterns_to_index =
      extract_and_normalize_patterns(raw_prices, window_size);

  size_t num_patterns = patterns_to_index.size();

  if (num_patterns == 0) {
    std::cerr << "Error: Not enough raw data to create any index patterns."
              << std::endl;
    return;
  }

  // 2. Create the Index using std::make_unique (L2 distance, Flat storage)
  index = std::make_unique<faiss::IndexFlatL2>(D);

  // 3. Prepare data for FAISS (Flatten the 2D vector into a 1D array)
  std::vector<float> all_data;
  all_data.reserve(num_patterns * D);

  for (const auto &pattern : patterns_to_index) {
    if (pattern.size() != D)
      continue;
    all_data.insert(all_data.end(), pattern.begin(), pattern.end());
  }

  // 4. Add vectors to the index
  index->add(all_data.size() / D, all_data.data());

  std::cout << "Indexing complete. Total vectors indexed: " << index->ntotal
            << std::endl;
}

void FaissIndexBackend::save_index(const std::string &index_path) {
  if (!index) {
    std::cerr << "Error: No index to save. Build an index first." << std::endl;
    return;
  }

  try {
    faiss::write_index(index.get(), index_path.c_str());
    std::cout << "Successfully saved FAISS index to " << index_path
              << std::endl;
  } catch (const faiss::FaissException &ex) {
    std::cerr << "Error saving FAISS index: " << ex.what() << std::endl;
  }
}

void FaissIndexBackend::load_index(const std::string &index_path,
                                   int window_size) {
  try {
    faiss::Index *raw_index = faiss::read_index(index_path.c_str());

    if (raw_index->d != window_size) {
      std::cerr << "Error: Loaded index dimension (" << raw_index->d
                << ") does not match window size (" << window_size << ")."
                << std::endl;
      delete raw_index;
      return;
    }

    // We assume the stored index is a FlatL2. We could dynamic_cast if more
    // types are supported.
    auto *typed_index = dynamic_cast<faiss::IndexFlatL2 *>(raw_index);
    if (!typed_index) {
      std::cerr << "Error: Loaded index is not an IndexFlatL2." << std::endl;
      delete raw_index;
      return;
    }

    // Transfer ownership
    index.reset(typed_index);
    std::cout << "Successfully loaded FAISS index with " << index->ntotal
              << " vectors." << std::endl;
  } catch (const faiss::FaissException &ex) {
    std::cerr << "Error loading FAISS index: " << ex.what() << std::endl;
  }
}

FaissSearchResults
FaissIndexBackend::search(const std::vector<float> &raw_query_pattern,
                          int k_neighbors, int window_size) {
  const int D = window_size;

  FaissSearchResults empty_results = {{}, {}};

  if (!index || index->ntotal == 0 || raw_query_pattern.size() != D) {
    std::cerr << "Error: Invalid index or query pattern size ("
              << raw_query_pattern.size() << " vs " << D << ")." << std::endl;
    return empty_results;
  }

  // 1. Preprocessing: Normalize the raw query pattern
  std::vector<float> query_pattern = normalize_window(raw_query_pattern);

  // 2. Search Preparation
  int k = std::min((int)index->ntotal, k_neighbors);
  std::vector<long> I(k); // faiss::idx_t is typically long
  std::vector<float> D_dist(k);

  // 3. Search
  index->search(1, query_pattern.data(), k, D_dist.data(), I.data());

  return {D_dist, I};
}
