#include "engine.h"
#include <numeric>
#include <cmath>
#include <iostream>
#include <algorithm> // for std::min

// Implementation of Z-Score Normalization
std::vector<float> normalize_window(const std::vector<float>& window) {
    // Note: Use 'double' for mean/stddev calculation to minimize floating point error
    if (window.empty()) return {};
    
    double sum = std::accumulate(window.begin(), window.end(), 0.0);
    double mean = sum / window.size();
    
    double sq_sum = 0.0;
    for (float val : window) sq_sum += (val - mean) * (val - mean);
    double std_dev = std::sqrt(sq_sum / window.size());
    
    // Avoid division by zero by setting std_dev to 1.0 (results in all zeros for normalized data)
    if (std_dev < 1e-5) std_dev = 1.0; 

    std::vector<float> normalized;
    normalized.reserve(window.size());
    for (float val : window) {
        normalized.push_back((float)((val - mean) / std_dev));
    }
    return normalized;
}

// Function to split the price series into normalized, overlapping patterns
std::vector<std::vector<float>> extract_and_normalize_patterns(
    const std::vector<float>& prices,
    int window_size
) {
    std::vector<std::vector<float>> patterns;
    if (prices.size() < (size_t)window_size) return patterns;

    // FIX: Index ALL windows in the provided prices vector, as the query is now handled externally.
    // The loop runs until the window starts at the (prices.size() - window_size) index.
    for (size_t i = 0; i <= prices.size() - window_size; ++i) { 
        std::vector<float> window(prices.begin() + i, prices.begin() + i + window_size);
        patterns.push_back(normalize_window(window));
    }
    return patterns;
}

// --- FAISS Indexing Function ---

std::unique_ptr<faiss::IndexFlatL2> create_faiss_index(
    const std::vector<float>& raw_prices,
    int window_size
) {
    const int D = window_size; 
    
    // 1. Preprocessing: Extract and Normalize all index patterns
    auto patterns_to_index = extract_and_normalize_patterns(raw_prices, window_size);

    size_t num_patterns = patterns_to_index.size();

    if (num_patterns == 0) {
        std::cerr << "Error: Not enough raw data to create any index patterns." << std::endl;
        return nullptr;
    }

    // 2. Create the Index using std::make_unique (L2 distance, Flat storage)
    std::unique_ptr<faiss::IndexFlatL2> index = std::make_unique<faiss::IndexFlatL2>(D);
    
    // 3. Prepare data for FAISS (Flatten the 2D vector into a 1D array)
    std::vector<float> all_data;
    all_data.reserve(num_patterns * D);
    
    for (const auto& pattern : patterns_to_index) {
        if (pattern.size() != D) continue; 
        all_data.insert(all_data.end(), pattern.begin(), pattern.end());
    }

    // 4. Add vectors to the index
    index->add(all_data.size() / D, all_data.data());

    std::cout << "Indexing complete. Total vectors indexed: " << index->ntotal << std::endl;
    
    return index;
}

// --- FAISS Query Function (No change needed) ---

FaissSearchResults run_faiss_query(
    const std::unique_ptr<faiss::IndexFlatL2>& index, 
    const std::vector<float>& raw_query_pattern,
    int k_neighbors,
    int window_size
) {
    const int D = window_size;
    
    FaissSearchResults empty_results = {{}, {}};

    if (!index || index->ntotal == 0 || raw_query_pattern.size() != D) { 
        std::cerr << "Error: Invalid index or query pattern size (" << raw_query_pattern.size() << " vs " << D << ")." << std::endl;
        return empty_results;
    }
    
    // 1. Preprocessing: Normalize the raw query pattern
    std::vector<float> query_pattern = normalize_window(raw_query_pattern);
    
    // 2. Search Preparation
    int k = std::min((int)index->ntotal, k_neighbors);
    std::vector<faiss::idx_t> I(k);  
    std::vector<float> D_dist(k);   

    // 3. Search
    index->search(1, query_pattern.data(), k, D_dist.data(), I.data()); 
    
    return {D_dist, I};
}