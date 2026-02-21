cmake_minimum_required(VERSION 3.10)
project(FinancialPatternSearch)

set(CMAKE_CXX_STANDARD 17)

# 1. Find the necessary libraries and headers in standard paths (/usr/local)
find_package(BLAS REQUIRED)
find_library(FAISS_LIB faiss)
find_path(FAISS_INCLUDE faiss/IndexFlat.h)
find_package(GTest REQUIRED) 

if(NOT FAISS_LIB OR NOT FAISS_INCLUDE)
    message(FATAL_ERROR "FAISS headers or library not found. Installation may have failed.")
endif()

include_directories(${FAISS_INCLUDE})

# Define the source files (All in the root directory)
set(ENGINE_SOURCES engine.cpp faiss_index_backend.cpp)
set(PARSER_SOURCES csv_parser.cpp)
set(ALL_SOURCES main.cpp ${ENGINE_SOURCES} ${PARSER_SOURCES})
set(TEST_SOURCES engine_test.cpp csv_parser_test.cpp faiss_index_backend_test.cpp)

# 2. Define the main executable
add_executable(fin_search ${ALL_SOURCES})

# 3. Link libraries for the main executable
target_link_libraries(fin_search PRIVATE ${FAISS_LIB} ${BLAS_LIBRARIES} pthread)

# --- UNIT TESTS ---
# 4. Define the test executable
# All source files are linked here so tests can call functions from engine.cpp and csv_parser.cpp
add_executable(fin_search_test ${TEST_SOURCES} ${ENGINE_SOURCES} ${PARSER_SOURCES}) 

# 5. Link libraries for the test executable
target_link_libraries(fin_search_test PRIVATE 
    GTest::gtest_main 
    ${FAISS_LIB}
    ${BLAS_LIBRARIES} 
    pthread
)

# 6. Enable CTest support and discovery
include(GoogleTest)
add_custom_target(run_test_discovery
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/gtest_discovery_ran
    COMMENT "Forcing GoogleTest discovery."
)

gtest_discover_tests(fin_search_test
    DEPENDS fin_search_test run_test_discovery
)

# 7. Install rules
install(TARGETS fin_search fin_search_test DESTINATION bin)