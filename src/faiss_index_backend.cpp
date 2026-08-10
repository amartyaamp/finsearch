#include "faiss_index_backend.h"
#include "engine.h"
#include <fstream>
#include <iostream>

void FaissIndexBackend::build_index(
    const std::vector<std::vector<float>> &feature_series,
    const std::vector<RowMetadata> &metadata, int window_size) {
  if (feature_series.empty())
    return;
  const int D = window_size * feature_series.size();

  // 1. Preprocessing: Extract and Normalize all index patterns
  auto patterns_to_index =
      extract_and_normalize_multivariate_patterns(feature_series, window_size);

  size_t num_patterns = patterns_to_index.size();

  if (num_patterns == 0) {
    std::cerr << "Error: Not enough raw data to create any index patterns."
              << std::endl;
    return;
  }

  // 2. Create the Index using std::make_unique (L2 distance, Flat storage) if it doesn't exist
  if (!index) {
    index = std::make_unique<faiss::IndexFlatL2>(D);
    metadata_store.clear();
  }

  // 3. Prepare data for FAISS (Flatten the 2D vector into a 1D array)
  std::vector<float> all_data;
  all_data.reserve(num_patterns * D);

  for (size_t i = 0; i < patterns_to_index.size(); ++i) {
    const auto &pattern = patterns_to_index[i];
    if (pattern.size() != D)
      continue;
    all_data.insert(all_data.end(), pattern.begin(), pattern.end());

    // Store metadata for this window
    // Window `i` in `patterns_to_index` corresponds to raw_prices from `i` to
    // `i + window_size - 1`
    if (i + window_size - 1 < metadata.size()) {
      WindowMetadata win_meta;
      win_meta.start_date = metadata[i].timestamp;
      win_meta.end_date = metadata[i + window_size - 1].timestamp;
      metadata_store.push_back(win_meta);
    }
  }

  // 4. Add vectors to the index
  index->add(all_data.size() / D, all_data.data());

  // Handle case where we skipped patterns
  if (metadata_store.size() != index->ntotal) {
    std::cerr << "Warning: Metadata count (" << metadata_store.size()
              << ") does not match index count (" << index->ntotal << ")."
              << std::endl;
  }

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

    // Save metadata
    std::string meta_path = index_path + ".meta";
    std::ofstream meta_file(meta_path);
    if (meta_file.is_open()) {
      for (const auto &meta : metadata_store) {
        meta_file << meta.start_date << "," << meta.end_date << "\n";
      }
      std::cout << "Successfully saved metadata to " << meta_path << std::endl;
    } else {
      std::cerr << "Error: Could not open metadata file for writing: "
                << meta_path << std::endl;
    }

  } catch (const faiss::FaissException &ex) {
    std::cerr << "Error saving FAISS index: " << ex.what() << std::endl;
  }
}

void FaissIndexBackend::load_index(const std::string &index_path,
                                   int window_size) {
  try {
    faiss::Index *raw_index = faiss::read_index(index_path.c_str());

    if (raw_index->d % window_size != 0) {
      std::cerr << "Error: Loaded index dimension (" << raw_index->d
                << ") is not a multiple of window size (" << window_size << ")."
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

    // Load metadata
    metadata_store.clear();
    std::string meta_path = index_path + ".meta";
    std::ifstream meta_file(meta_path);
    if (meta_file.is_open()) {
      std::string line;
      while (std::getline(meta_file, line)) {
        size_t comma_pos = line.find(',');
        if (comma_pos != std::string::npos) {
          WindowMetadata meta;
          meta.start_date = line.substr(0, comma_pos);
          meta.end_date = line.substr(comma_pos + 1);
          metadata_store.push_back(meta);
        }
      }
      std::cout << "Successfully loaded " << metadata_store.size()
                << " metadata entries." << std::endl;

      if (metadata_store.size() != index->ntotal) {
        std::cerr << "Warning: Loaded metadata count (" << metadata_store.size()
                  << ") does not match index count (" << index->ntotal << ")."
                  << std::endl;
      }

    } else {
      std::cerr << "Warning: Could not open metadata file for reading: "
                << meta_path << std::endl;
    }

  } catch (const faiss::FaissException &ex) {
    std::cerr << "Error loading FAISS index: " << ex.what() << std::endl;
  }
}

std::vector<SearchResult> FaissIndexBackend::search(
    const std::vector<std::vector<float>> &raw_query_series, int k_neighbors,
    int window_size) {
  if (raw_query_series.empty())
    return {};
  const int D = window_size * raw_query_series.size();

  std::vector<SearchResult> results;

  if (!index || index->ntotal == 0 ||
      raw_query_series[0].size() != window_size) {
    std::cerr << "Error: Invalid index or query pattern size." << std::endl;
    return results;
  }

  // 1. Preprocessing: Normalize the raw query pattern
  std::vector<float> query_pattern;
  query_pattern.reserve(D);
  for (const auto &series : raw_query_series) {
    auto norm_window = normalize_window(series);
    query_pattern.insert(query_pattern.end(), norm_window.begin(),
                         norm_window.end());
  }

  // 2. Search Preparation
  int k = std::min((int)index->ntotal, k_neighbors);
  std::vector<long> I(k); // faiss::idx_t is typically long
  std::vector<float> D_dist(k);

  // 3. Search
  index->search(1, query_pattern.data(), k, D_dist.data(), I.data());

  // 4. Package Results
  for (int i = 0; i < k; ++i) {
    SearchResult res;
    res.distance = D_dist[i];
    res.confidence_score = compute_confidence_score(D_dist[i], (float)D);
    res.index = I[i];
    if (res.index >= 0 && res.index < metadata_store.size()) {
      res.metadata = metadata_store[res.index];
    } else {
      res.metadata = {"UNKNOWN", "UNKNOWN"};
    }
    results.push_back(res);
  }

  return results;
}
