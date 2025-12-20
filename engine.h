#pragma once
#include <vector>
#include <memory>
#include <faiss/Index.h> 
#include <faiss/IndexFlat.h> 

using FaissSearchResults = std::pair<std::vector<float>, std::vector<faiss::idx_t>>;

// Core logic for Z-Score Normalization
std::vector<float> normalize_window(const std::vector<float>& window);

// Function to extract ALL normalized index patterns from a raw price series
std::vector<std::vector<float>> extract_and_normalize_patterns(
    const std::vector<float>& prices, 
    int window_size 
);

// Function to create and train the FAISS index
std::unique_ptr<faiss::IndexFlatL2> create_faiss_index(
    const std::vector<float>& raw_prices,
    int window_size 
);

// Function to run the FAISS query
FaissSearchResults run_faiss_query(
    const std::unique_ptr<faiss::IndexFlatL2>& index, 
    const std::vector<float>& raw_query_pattern,
    int k_neighbors,
    int window_size 
);