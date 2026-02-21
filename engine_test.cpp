#include "engine.h"
#include "faiss_index_backend.h"
#include "gtest/gtest.h"
#include <cmath>
#include <numeric>

// Define a small window size for manual verification of normalization and
// distance
const int D_TEST = 3;

// Helper function to create the raw price history data.
// It ONLY contains the patterns we want indexed (P_Far and P_Near).
std::vector<float> create_raw_data_for_test(const std::vector<float> &p_near,
                                            const std::vector<float> &p_far) {
  std::vector<float> raw_prices;
  // P_Far (elements 0-2 of raw_prices) -> Indexed at index 0
  raw_prices.insert(raw_prices.end(), p_far.begin(), p_far.end());
  // P_Near (elements 3-5 of raw_prices) -> Indexed at index 3 (due to overlaps)
  raw_prices.insert(raw_prices.end(), p_near.begin(), p_near.end());

  return raw_prices; // Total size is now 6
}

TEST(EngineTest, QueryReturnsNearestNeighbor) {
  // 1. Define Raw Patterns (using D_TEST = 3)

  // Pattern A (NEAR, Expected Index 3): Linear shape (10, 20, 30)
  std::vector<float> raw_pattern_near = {10.0f, 20.0f, 30.0f};

  // Pattern B (FAR, Expected Index 0): Flat shape (100, 100, 100)
  std::vector<float> raw_pattern_far = {100.0f, 100.0f, 100.0f};

  // Query Pattern (Q): Same linear shape as NEAR (5, 15, 25). Distance to
  // P_Near is 0. This is explicitly kept separate from the history.
  std::vector<float> raw_query_pattern = {5.0f, 15.0f, 25.0f};

  // Create the full raw price history: {P_Far, P_Near}. Total size = 6.
  // The query pattern is NOT included here.
  std::vector<float> raw_prices =
      create_raw_data_for_test(raw_pattern_near, raw_pattern_far);

  // Create dummy metadata to satisfy the new interface
  std::vector<RowMetadata> dummy_meta(raw_prices.size());

  // 2. Index Creation (PASS D_TEST)
  FaissIndexBackend backend;
  backend.build_index(raw_prices, dummy_meta, D_TEST);

  // FIX: Total number of indexable windows is now (N - D + 1) = (6 - 3 + 1)
  // = 4. The indexed patterns start at indices 0, 1, 2, 3.
  ASSERT_EQ(backend.get_total_vectors(), 4);

  // 3. Query Execution (PASS D_TEST)
  int k_neighbors = 2;
  // The run_faiss_query function receives the separate, D-dimensional query
  // pattern.
  auto results = backend.search(raw_query_pattern, k_neighbors, D_TEST);

  // 4. Assertions
  ASSERT_EQ(results.size(), k_neighbors);

  // --- ASSERTION 1 (Nearest Neighbor: Index 3) ---
  // The P_Near pattern starts at the 4th element (index 3) in the 6-element
  // history.
  ASSERT_EQ(results[0].index, 3);
  ASSERT_NEAR(results[0].distance, 0.0f, 1e-4);

  // --- ASSERTION 2 (Second Nearest Neighbor: Index 0) ---
  // Expected squared distance is D_TEST = 3.0f.
  const float expected_far_squared_distance = (float)D_TEST;

  ASSERT_EQ(results[1].index, 0); // The P_Far pattern is indexed at index 0.
  ASSERT_NEAR(results[1].distance, expected_far_squared_distance, 1e-4);
}