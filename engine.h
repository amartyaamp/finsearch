#pragma once
#include <vector>
using FaissSearchResults = std::pair<std::vector<float>, std::vector<long>>;

// Core logic for Z-Score Normalization
std::vector<float> normalize_window(const std::vector<float> &window);

// Function to extract ALL normalized index patterns from a raw price series
std::vector<std::vector<float>>
extract_and_normalize_patterns(const std::vector<float> &prices,
                               int window_size);
