# Project Roadmap & TODO

This document tracks the planned features and improvements for the FinSearch (Time Series Pattern Matcher) project.

## 🚀 Core Features
- [ ] **Configurable Parameters:** Allow `WINDOW_SIZE` and `K_NEIGHBORS` to be passed as command-line arguments.
- [ ] **Extended Distance Metrics:** Implement support for Inner Product and Cosine Similarity in addition to L2.
- [ ] **Normalization Strategies:** Add Min-Max scaling and Robust Scaler options.
- [ ] **Multivariate Support:** Enable pattern matching across multiple time series (e.g., Price + Volume).
- [ ] **Metadata Tagging:** Store and return timestamps/dates along with the index positions for better result interpretation.

## 🏗 Architectural Refactoring
- [ ] **Decoupled Indexing Flow:** Create a separate flow/utility for indexing. This process will read the entire historical dataset, create the FAISS index, and persist the created index to disk (e.g., as a `.index` file) along with necessary metadata.
- [ ] **Decoupled Query Flow:** Create a separate flow/utility for querying. This process will load the persisted index from disk and perform fast lookups without needing to re-index the historical data.
- [ ] **Index Versioning:** Store metadata (WINDOW_SIZE, normalization type) alongside the persisted index to ensure consistency during lookups.

## 🚀 Service & API Layer
- [ ] **Service/API Support:** Create a service or API layer (e.g., using Crow, gRPC, or a Python/Flask wrapper) that utilizes the C++ libraries built so far to expose indexing and querying capabilities over a network.
- [ ] **Library Packaging:** Refactor the core engine into a clean shared library (`.so` / `.dylib`) for easier integration into broader financial systems and the API layer.
- [ ] **C++ Service Core:** Develop a service wrapper around the existing libraries to handle concurrent search requests.

## 🛠 Robustness & Testing
- [ ] **Advanced CSV Parsing:** Handle malformed CSV files, missing values, and different date formats gracefully.
- [ ] **Validation Layer:** Add input validation for query patterns and historical data ranges.
- [ ] **Expanded Test Suite:** 
    - [ ] Unit tests for edge cases in `normalize_window`.
    - [ ] Mock tests for FAISS index creation failure scenarios.
- [x] **CI/CD Integration:** Set up GitHub Actions to run tests and build Docker images on every push.

## ⚡ Performance
- [ ] **FAISS-GPU:** Add support for GPU-accelerated searches for significantly larger datasets.
- [ ] **Memory Management:** Optimize memory consumption when loading very large historical CSVs.

## 🌐 UI & Integration
- [ ] **Visualization Dashboard:** Build a simple web interface (React/Next.js) to display query patterns and their nearest matches side-by-side using charts.
- [ ] **Live Data Sources:** Implement connectors for real-time financial data APIs (e.g., Alpha Vantage, Polygon.io).

## 📄 Documentation
- [ ] **API Documentation:** Generate Doxygen or similar documentation for the C++ core.
- [ ] **Architectural Overview:** Document the decoupled indexing/querying flow for future maintainers.
