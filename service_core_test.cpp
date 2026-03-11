#include "service_core.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include "csv_parser.h" // For RowMetadata
#include <thread>
#include <vector>
#include <atomic>

TEST(ServiceCoreTest, ConcurrentReads) {
    FinSearchService service;
    
    std::vector<std::vector<float>> feature_series = {
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}
    };
    std::vector<RowMetadata> metadata = {
        {"Day1"}, {"Day2"}, {"Day3"}, {"Day4"}, {"Day5"}, {"Day6"}
    };
    int window_size = 3;
    
    service.build_index(feature_series, metadata, window_size);
    EXPECT_EQ(service.get_total_vectors(), 4);
    
    std::atomic<int> success_count{0};

    auto worker = [&]() {
        std::vector<std::vector<float>> query = { {1.0f, 2.0f, 3.0f} };
        auto results = service.search(query, 2, window_size);
        if (results.size() == 2) {
            success_count++;
        }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back(worker);
    }
    
    for (auto &t : threads) {
        t.join();
    }

    EXPECT_EQ(success_count.load(), 20);
}
