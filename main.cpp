#include <algorithm>
#include <filesystem> // For checking file existence
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "csv_parser.h" // Data loading logic
#include "engine.h"     // Core FAISS and normalization logic

// CONFIGURATION
const int WINDOW_SIZE = 60;
const int K_NEIGHBORS = 5;
const std::string DEFAULT_HISTORY_FILE = "/app/data/nifty_50.csv";
const std::string DEFAULT_QUERY_FILE = "/app/data/sample_queries.csv";

void print_usage(const char *prog_name) {
  std::cout << "Usage: " << prog_name << " [query_csv_path]\n";
  std::cout << "  If no query file is provided, defaults to: "
            << DEFAULT_QUERY_FILE << "\n";
}

int main(int argc, char *argv[]) {
  // --- Phase 1: Load Historical Data (The Database) ---
  std::cout << "[1] Loading Historical Data from: " << DEFAULT_HISTORY_FILE
            << std::endl;
  auto history_prices = load_csv_close_prices(DEFAULT_HISTORY_FILE);

  if (history_prices.size() < WINDOW_SIZE) {
    std::cerr
        << "Error: History file has not enough data points! Need at least "
        << WINDOW_SIZE << "." << std::endl;
    return 1;
  }
  std::cout << "    Loaded " << history_prices.size() << " historical points."
            << std::endl;

  // --- Phase 2: Load Query Data (The Pattern to Search) ---
  std::string query_file_path = (argc > 1) ? argv[1] : DEFAULT_QUERY_FILE;
  std::cout << "[2] Loading Query Data from: " << query_file_path << std::endl;

  // Check if query file exists
  if (!std::filesystem::exists(query_file_path)) {
    std::cerr << "Error: Query file not found at " << query_file_path
              << std::endl;
    std::cerr << "Please create this file or provide a path as an argument."
              << std::endl;
    return 1;
  }

  auto query_prices = load_csv_close_prices(query_file_path);

  if (query_prices.size() < WINDOW_SIZE) {
    std::cerr << "Error: Query file too short. Need at least " << WINDOW_SIZE
              << " rows to form a pattern." << std::endl;
    return 1;
  }
  std::cout << "    Loaded " << query_prices.size() << " query points."
            << std::endl;

  // Extract the LAST window from the query file as our target pattern
  std::vector<float> raw_query_pattern(query_prices.end() - WINDOW_SIZE,
                                       query_prices.end());

  // --- Phase 3: Create Index ---
  std::cout << "[3] Creating FAISS Index from History..." << std::endl;

  // HACK: The create_faiss_index function in engine.cpp skips the LAST window
  // because it assumes that window is the query.
  // Since our query is external, we want to index the FULL history.
  // We duplicate the last price point so the engine "skips" this dummy point
  // and indexes the actual last window of history.
  history_prices.push_back(history_prices.back());

  std::unique_ptr<faiss::IndexFlatL2> index =
      create_faiss_index(history_prices, WINDOW_SIZE);

  if (!index || index->ntotal == 0) {
    std::cerr << "Error: Failed to create index." << std::endl;
    return 1;
  }

  // --- Phase 4: Run Query ---
  std::cout << "[4] Searching for similar patterns..." << std::endl;

  auto [distances, indices] =
      run_faiss_query(index, raw_query_pattern, K_NEIGHBORS, WINDOW_SIZE);

  // --- Phase 5: Output Results ---
  if (!indices.empty()) {
    std::cout << "\n=== Search Results (" << indices.size()
              << " Nearest Neighbors) ===" << std::endl;
    for (size_t i = 0; i < indices.size(); i++) {
      std::cout << "Rank " << i + 1 << " | Index: " << indices[i]
                << " | Distance: " << distances[i] << std::endl;
      // Note: 'Index' here refers to the row number in the nifty_50.csv file
      // where the similar pattern STARTs.
    }
    std::cout << "=================================================\n";
  } else {
    std::cerr << "Search returned no results." << std::endl;
  }

  return 0;
}