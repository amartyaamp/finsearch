#pragma once
#include <vector>
using FaissSearchResults = std::pair<std::vector<float>, std::vector<long>>;

// Core logic for Z-Score Normalization
std::vector<float> normalize_window(const std::vector<float> &window);

// Function to extract ALL normalized index patterns from a raw price series
std::vector<std::vector<float>>
extract_and_normalize_patterns(const std::vector<float> &prices,
                               int window_size);

// Map an L2 distance to a 0-100% confidence/similarity score.
// Uses exponential decay: 100 * exp(-distance / scale).
// A perfect match (distance=0) yields 100%; score decays smoothly as distance
// grows. `scale` should be set to the vector dimension (window_size).
float compute_confidence_score(float distance, float scale);
