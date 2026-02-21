#include "faiss_index_backend.h"
#include <cstdio> // for std::remove
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class FaissIndexBackendTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create some dummy synthetic price data
    for (int i = 0; i < 200; ++i) {
      dummy_prices.push_back(100.0f + i * 0.5f);
    }
  }

  void TearDown() override {
    // Cleanup the temporary file if it exists
    if (std::filesystem::exists(temp_index_path)) {
      std::filesystem::remove(temp_index_path);
    }
  }

  std::vector<float> dummy_prices;
  const std::string temp_index_path = "test_index.index";
  const int window_size = 10;
};

TEST_F(FaissIndexBackendTest, BuildIndexStoresVectors) {
  FaissIndexBackend backend;
  backend.build_index(dummy_prices, window_size);
  // For 200 points and window 10, extract_and_normalize_patterns gives 191
  // patterns.
  EXPECT_EQ(backend.get_total_vectors(), 191);
}

TEST_F(FaissIndexBackendTest, SaveAndLoadMaintainsData) {
  // 1. Build and save
  {
    FaissIndexBackend backend;
    backend.build_index(dummy_prices, window_size);
    EXPECT_EQ(backend.get_total_vectors(), 191);
    backend.save_index(temp_index_path);
    EXPECT_TRUE(std::filesystem::exists(temp_index_path));
  }

  // 2. Load into new instance
  {
    FaissIndexBackend loader_backend;
    loader_backend.load_index(temp_index_path, window_size);
    EXPECT_EQ(loader_backend.get_total_vectors(), 191);
  }
}

TEST_F(FaissIndexBackendTest, LoadFailsOnDimensionMismatch) {
  FaissIndexBackend backend;
  backend.build_index(dummy_prices, window_size);
  backend.save_index(temp_index_path);

  FaissIndexBackend loader_backend;
  testing::internal::CaptureStderr();
  // Try to load a window_size=10 index with window_size=20
  loader_backend.load_index(temp_index_path, 20);
  std::string output = testing::internal::GetCapturedStderr();

  EXPECT_NE(output.find("does not match window size"), std::string::npos);
  EXPECT_EQ(loader_backend.get_total_vectors(), 0);
}

TEST_F(FaissIndexBackendTest, SearchReturnsValidResults) {
  FaissIndexBackend backend;
  backend.build_index(dummy_prices, window_size);

  // Create a target query matching the very first window (normalized)
  std::vector<float> target_query(dummy_prices.begin(),
                                  dummy_prices.begin() + window_size);

  auto results = backend.search(target_query, 3, window_size);
  auto distances = results.first;
  auto indices = results.second;

  ASSERT_EQ(distances.size(), 3);
  ASSERT_EQ(indices.size(), 3);

  // The closest match should be exactly the first pattern at index 0
  // whose distance to itself should be 0 (or close to floating point precision)
  EXPECT_EQ(indices[0], 0);
  EXPECT_LE(distances[0], 1e-5);
}
