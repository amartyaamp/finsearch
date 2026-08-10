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
      RowMetadata meta;
      meta.timestamp = "2023-01-01 " + std::to_string(i) + ":00:00";
      dummy_metadata.push_back(meta);
    }
  }

  void TearDown() override {
    // Cleanup the temporary file if it exists
    if (std::filesystem::exists(temp_index_path)) {
      std::filesystem::remove(temp_index_path);
    }
    std::string temp_meta_path = temp_index_path + ".meta";
    if (std::filesystem::exists(temp_meta_path)) {
      std::filesystem::remove(temp_meta_path);
    }
  }

  std::vector<float> dummy_prices;
  std::vector<RowMetadata> dummy_metadata;
  const std::string temp_index_path = "test_index.index";
  const int window_size = 10;
};

TEST_F(FaissIndexBackendTest, BuildIndexStoresVectors) {
  FaissIndexBackend backend;
  backend.build_index({dummy_prices}, dummy_metadata, window_size);
  // For 200 points and window 10, extract_and_normalize_patterns gives 191
  // patterns.
  EXPECT_EQ(backend.get_total_vectors(), 191);
  EXPECT_EQ(backend.get_total_metadata(), 191);
}

TEST_F(FaissIndexBackendTest, SaveAndLoadMaintainsData) {
  // 1. Build and save
  {
    FaissIndexBackend backend;
    backend.build_index({dummy_prices}, dummy_metadata, window_size);
    EXPECT_EQ(backend.get_total_vectors(), 191);
    backend.save_index(temp_index_path);
    bool index_exists = std::filesystem::exists(temp_index_path);
    bool meta_exists = std::filesystem::exists(temp_index_path + ".meta");
    EXPECT_TRUE(index_exists);
    EXPECT_TRUE(meta_exists);
  }

  // 2. Load into new instance
  {
    FaissIndexBackend loader_backend;
    loader_backend.load_index(temp_index_path, window_size);
    EXPECT_EQ(loader_backend.get_total_vectors(), 191);
    EXPECT_EQ(loader_backend.get_total_metadata(), 191);
  }
}

TEST_F(FaissIndexBackendTest, LoadFailsOnDimensionMismatch) {
  FaissIndexBackend backend;
  backend.build_index({dummy_prices}, dummy_metadata, window_size);
  backend.save_index(temp_index_path);

  FaissIndexBackend loader_backend;
  testing::internal::CaptureStderr();
  // Try to load a window_size=10 index with window_size=20
  loader_backend.load_index(temp_index_path, 20);
  std::string output = testing::internal::GetCapturedStderr();

  EXPECT_NE(output.find("is not a multiple of window size"), std::string::npos);
  EXPECT_EQ(loader_backend.get_total_vectors(), 0);
}

TEST_F(FaissIndexBackendTest, SearchReturnsValidResults) {
  FaissIndexBackend backend;
  backend.build_index({dummy_prices}, dummy_metadata, window_size);

  // Create a target query matching the very first window (normalized)
  std::vector<float> target_query(dummy_prices.begin(),
                                  dummy_prices.begin() + window_size);

  auto results = backend.search({target_query}, 3, window_size);

  ASSERT_EQ(results.size(), 3);

  // The closest match should be exactly the first pattern at index 0
  // whose distance to itself should be 0 (or close to floating point precision)
  EXPECT_EQ(results[0].index, 0);
  EXPECT_LE(results[0].distance, 1e-5);
  EXPECT_EQ(results[0].metadata.start_date, dummy_metadata[0].timestamp);
}

TEST_F(FaissIndexBackendTest, SearchIncludesConfidenceScore) {
  FaissIndexBackend backend;
  backend.build_index({dummy_prices}, dummy_metadata, window_size);

  // Query with the very first window — it is an exact match of itself
  std::vector<float> target_query(dummy_prices.begin(),
                                  dummy_prices.begin() + window_size);
  auto results = backend.search({target_query}, 5, window_size);

  ASSERT_FALSE(results.empty());

  for (const auto &res : results) {
    // Every confidence score must be in [0, 100]
    EXPECT_GE(res.confidence_score, 0.0f);
    EXPECT_LE(res.confidence_score, 100.0f);
  }

  // The top result is an exact self-match (distance ≈ 0) → must be ≥ 99%
  EXPECT_GE(results[0].confidence_score, 99.0f);
}

TEST_F(FaissIndexBackendTest, AppendsToExistingIndex) {
  FaissIndexBackend backend;
  // First build
  backend.build_index({dummy_prices}, dummy_metadata, window_size);
  long initial_vectors = backend.get_total_vectors();
  EXPECT_EQ(initial_vectors, 191);
  
  // Create another set of dummy data to append
  std::vector<float> more_prices;
  std::vector<RowMetadata> more_metadata;
  for (int i = 0; i < 50; ++i) {
    more_prices.push_back(200.0f + i * 0.5f);
    RowMetadata meta;
    meta.timestamp = "2023-02-01 " + std::to_string(i) + ":00:00";
    more_metadata.push_back(meta);
  }
  
  // Second build (should append)
  backend.build_index({more_prices}, more_metadata, window_size);
  long final_vectors = backend.get_total_vectors();
  long expected_additional = 50 - window_size + 1; // 41
  
  EXPECT_EQ(final_vectors, initial_vectors + expected_additional);
  EXPECT_EQ(backend.get_total_metadata(), initial_vectors + expected_additional);
}

