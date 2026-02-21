#include "engine.h"
#include <algorithm> // for std::min
#include <cmath>
#include <iostream>
#include <numeric>

// Implementation of Z-Score Normalization
std::vector<float> normalize_window(const std::vector<float> &window) {
  // Note: Use 'double' for mean/stddev calculation to minimize floating point
  // error
  if (window.empty())
    return {};

  double sum = std::accumulate(window.begin(), window.end(), 0.0);
  double mean = sum / window.size();

  double sq_sum = 0.0;
  for (float val : window)
    sq_sum += (val - mean) * (val - mean);
  double std_dev = std::sqrt(sq_sum / window.size());

  // Avoid division by zero by setting std_dev to 1.0 (results in all zeros for
  // normalized data)
  if (std_dev < 1e-5)
    std_dev = 1.0;

  std::vector<float> normalized;
  normalized.reserve(window.size());
  for (float val : window) {
    normalized.push_back((float)((val - mean) / std_dev));
  }
  return normalized;
}

// Function to split the price series into normalized, overlapping patterns
std::vector<std::vector<float>>
extract_and_normalize_patterns(const std::vector<float> &prices,
                               int window_size) {
  std::vector<std::vector<float>> patterns;
  if (prices.size() < (size_t)window_size)
    return patterns;

  // FIX: Index ALL windows in the provided prices vector, as the query is now
  // handled externally. The loop runs until the window starts at the
  // (prices.size() - window_size) index.
  for (size_t i = 0; i <= prices.size() - window_size; ++i) {
    std::vector<float> window(prices.begin() + i,
                              prices.begin() + i + window_size);
    patterns.push_back(normalize_window(window));
  }
  return patterns;
}
