#include "csv_parser.h"
#include "faiss_index_backend.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

const int WINDOW_SIZE = 60;
const int K_NEIGHBORS = 5;

void print_usage(const char *prog_name) {
  std::cout << "Usage: " << prog_name << " <command> [arguments]\n";
  std::cout << "Commands:\n";
  std::cout << "  index  <history_csv_path> <output_index_path>\n";
  std::cout << "         Example: " << prog_name
            << " index /app/data/nifty_50.csv my_nifty.index\n";
  std::cout << "  search <input_index_path> <query_csv_path>\n";
  std::cout << "         Example: " << prog_name
            << " search my_nifty.index /app/data/sample_queries.csv\n";
}

int handle_index_command(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Error: 'index' command requires history_csv_path and "
                 "output_index_path.\n";
    print_usage(argv[0]);
    return 1;
  }

  std::string history_csv = argv[2];
  std::string output_index = argv[3];

  std::cout << "[1] Loading Historical Data from: " << history_csv << std::endl;
  auto data = load_csv_data(history_csv);
  auto &history_prices = data.prices;
  auto &history_metadata = data.row_metadata;

  if (history_prices.size() < WINDOW_SIZE) {
    std::cerr << "Error: History file (" << history_csv
              << ") has not enough data points! Need at least " << WINDOW_SIZE
              << "." << std::endl;
    return 1;
  }

  // HACK: Duplicating the last price point so the engine "skips" this dummy
  // point and indexes the actual last window of history. (Preserving prior
  // logic behavior)
  history_prices.push_back(history_prices.back());
  if (!history_metadata.empty()) {
    history_metadata.push_back(history_metadata.back()); // keep them parallel
  }

  std::cout << "[2] Creating FAISS Index..." << std::endl;
  FaissIndexBackend backend;
  backend.build_index(history_prices, history_metadata, WINDOW_SIZE);

  std::cout << "[3] Saving FAISS Index to: " << output_index << std::endl;
  backend.save_index(output_index);

  return 0;
}

int handle_search_command(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Error: 'search' command requires input_index_path and "
                 "query_csv_path.\n";
    print_usage(argv[0]);
    return 1;
  }

  std::string input_index = argv[2];
  std::string query_csv = argv[3];

  if (!std::filesystem::exists(query_csv)) {
    std::cerr << "Error: Query file not found at " << query_csv << std::endl;
    return 1;
  }
  if (!std::filesystem::exists(input_index)) {
    std::cerr << "Error: Index file not found at " << input_index << std::endl;
    return 1;
  }

  std::cout << "[1] Loading FAISS Index from: " << input_index << std::endl;
  FaissIndexBackend backend;
  backend.load_index(input_index, WINDOW_SIZE);

  if (backend.get_total_vectors() == 0) {
    std::cerr << "Error: Loaded index is empty or failed to load." << std::endl;
    return 1;
  }

  std::cout << "[2] Loading Query Data from: " << query_csv << std::endl;
  auto query_prices = load_csv_close_prices(query_csv);

  if (query_prices.size() < WINDOW_SIZE) {
    std::cerr << "Error: Query file too short. Need at least " << WINDOW_SIZE
              << " rows." << std::endl;
    return 1;
  }

  std::vector<float> raw_query_pattern(query_prices.end() - WINDOW_SIZE,
                                       query_prices.end());

  std::cout << "[3] Searching for similar patterns..." << std::endl;
  auto results = backend.search(raw_query_pattern, K_NEIGHBORS, WINDOW_SIZE);

  if (!results.empty()) {
    std::cout << "\n=== Search Results (" << results.size()
              << " Nearest Neighbors) ===\n";
    std::cout << std::fixed << std::setprecision(2);
    for (size_t i = 0; i < results.size(); i++) {
      std::cout << "Rank " << i + 1
                << " | Confidence: " << results[i].confidence_score << "%"
                << " | Distance: " << results[i].distance
                << " | Index: " << results[i].index
                << " | Dates: " << results[i].metadata.start_date << " to "
                << results[i].metadata.end_date << std::endl;
    }
    std::cout << "=================================================\n";
  } else {
    std::cerr << "Search returned no results." << std::endl;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::string command = argv[1];

  if (command == "index") {
    return handle_index_command(argc, argv);
  } else if (command == "search") {
    return handle_search_command(argc, argv);
  } else {
    std::cerr << "Error: Unknown command '" << command << "'\n";
    print_usage(argv[0]);
    return 1;
  }
}