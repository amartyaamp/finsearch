#pragma once
#include <vector>
#include <string>

// Function to load the Close prices from a simple financial CSV file.
std::vector<float> load_csv_close_prices(const std::string& filename);

// Function to load query patterns from a CSV file.
// Each row represents a separate query pattern.
std::vector<std::vector<float>> load_query_patterns(const std::string& filename);