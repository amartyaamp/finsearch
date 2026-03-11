#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// Opaque handle for the FinSearchService
typedef void* FinSearchServiceHandle;

// Result structure mapped safely to C
typedef struct {
    float distance;
    float confidence_score;
    long index;
    const char* start_date;
    const char* end_date;
} CSearchResult;

// Initialize a new thread-safe FinSearchService
FinSearchServiceHandle fin_search_service_create();

// Destroy the service and free memory
void fin_search_service_destroy(FinSearchServiceHandle handle);

// Build an index from a given CSV file directly
// Returns 0 on success, negative on error
int fin_search_service_build_index_from_csv(FinSearchServiceHandle handle, const char* csv_file_path, int window_size);

// Save the current index to disk
// Returns 0 on success, negative on error
int fin_search_service_save_index(FinSearchServiceHandle handle, const char* index_path);

// Load an existing index from disk
// Returns 0 on success, negative on error
int fin_search_service_load_index(FinSearchServiceHandle handle, const char* index_path, int window_size);

// Search the index.
// query_series: Array of float* pointers. query_series[i] is an array of size `window_size`.
// Returns an allocated array of CSearchResult. If none/error, returns NULL and sets out_count to 0.
// You MUST call fin_search_free_results on the returned pointer to avoid memory leaks.
CSearchResult* fin_search_service_search(
    FinSearchServiceHandle handle,
    const float** query_series,
    size_t num_features,
    size_t window_size,
    int k_neighbors,
    size_t* out_count
);

// Free an allocated CSearchResult array
void fin_search_free_results(CSearchResult* results, size_t count);

#ifdef __cplusplus
}
#endif
