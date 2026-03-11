#include "c_api.h"
#include "service_core.h"
#include "csv_parser.h"
#include "csv_data_source.h"
#include <cstring>
#include <iostream>

extern "C" {

FinSearchServiceHandle fin_search_service_create() {
    return new FinSearchService();
}

void fin_search_service_destroy(FinSearchServiceHandle handle) {
    if (handle) {
        delete static_cast<FinSearchService*>(handle);
    }
}

int fin_search_service_build_index_from_csv(FinSearchServiceHandle handle, const char* csv_file_path, int window_size) {
    if (!handle || !csv_file_path) return -1;
    try {
        FinSearchService* service = static_cast<FinSearchService*>(handle);
        auto data = load_csv_data(csv_file_path);
        
        if (data.features.empty() || data.features[0].size() < window_size) {
            return -2;
        }
        
        for (auto &series : data.features) {
            series.push_back(series.back());
        }
        if (!data.row_metadata.empty()) {
            data.row_metadata.push_back(data.row_metadata.back());
        }

        service->build_index(data.features, data.row_metadata, window_size);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C-API Exception during build_index: " << e.what() << std::endl;
        return -3;
    }
}

int fin_search_service_save_index(FinSearchServiceHandle handle, const char* index_path) {
    if (!handle || !index_path) return -1;
    try {
        FinSearchService* service = static_cast<FinSearchService*>(handle);
        service->save_index(index_path);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C-API Exception during save_index: " << e.what() << std::endl;
        return -2;
    }
}

int fin_search_service_load_index(FinSearchServiceHandle handle, const char* index_path, int window_size) {
    if (!handle || !index_path) return -1;
    try {
        FinSearchService* service = static_cast<FinSearchService*>(handle);
        service->load_index(index_path, window_size);
        if (service->get_total_vectors() == 0) return -2;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C-API Exception during load_index: " << e.what() << std::endl;
        return -3;
    }
}

CSearchResult* fin_search_service_search(
    FinSearchServiceHandle handle,
    const float** query_series,
    size_t num_features,
    size_t window_size,
    int k_neighbors,
    size_t* out_count
) {
    if (!handle || !query_series || !out_count) return nullptr;
    try {
        FinSearchService* service = static_cast<FinSearchService*>(handle);
        
        std::vector<std::vector<float>> cpp_query_series(num_features);
        for (size_t i = 0; i < num_features; ++i) {
            cpp_query_series[i].assign(query_series[i], query_series[i] + window_size);
        }

        auto results = service->search(cpp_query_series, k_neighbors, window_size);
        *out_count = results.size();

        if (results.empty()) {
            return nullptr;
        }

        CSearchResult* c_results = new CSearchResult[results.size()];
        for (size_t i = 0; i < results.size(); ++i) {
            c_results[i].distance = results[i].distance;
            c_results[i].confidence_score = results[i].confidence_score;
            c_results[i].index = results[i].index;
            
            c_results[i].start_date = strdup(results[i].metadata.start_date.c_str());
            c_results[i].end_date = strdup(results[i].metadata.end_date.c_str());
        }

        return c_results;

    } catch (const std::exception& e) {
        std::cerr << "C-API Exception during search: " << e.what() << std::endl;
        *out_count = 0;
        return nullptr;
    }
}

void fin_search_free_results(CSearchResult* results, size_t count) {
    if (!results) return;
    for (size_t i = 0; i < count; ++i) {
        if (results[i].start_date) free((void*)results[i].start_date);
        if (results[i].end_date) free((void*)results[i].end_date);
    }
    delete[] results;
}

} // extern "C"
