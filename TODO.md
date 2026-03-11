# Project Roadmap & TODO

This document tracks the planned features and improvements for the FinSearch (Time Series Pattern Matcher) project.

## 🚀 Core Features
- [ ] **Configurable Parameters:** Allow `WINDOW_SIZE` and `K_NEIGHBORS` to be passed as command-line arguments.
- [ ] **Extended Distance Metrics:** Implement support for Inner Product and Cosine Similarity in addition to L2.
- [ ] **Normalization Strategies:** Add Min-Max scaling and Robust Scaler options.
- [x] **Multivariate Support (MVP Critical):** Enable pattern matching across multiple time series (e.g., Price + Volume). *Crucial because pattern shapes mean little without the context of trading volume to confirm institutional participation.*
    - [ ] Future: Support assigning different weights for different features during distance calculation to fine-tune similarity importance.
- [x] **Metadata Tagging (MVP Critical):** Store and return timestamps/dates, and contextual metadata along with the index positions. *Crucial because knowing when a pattern occurred (e.g., 2008 crash vs. 2021 bull run) is required for strategy context.*
- [x] **Confidence Scoring (MVP Critical):** Map absolute vector distances to a normalized "Similarity/Confidence Score" (e.g., 0-100%). *Crucial because humans (retail traders) and external systems need an intuitive bounding box on how "exact" a match is, unlike rigid rule-based systems.*
- [x] **Arbitrary Data Sources:** Support time series data from any data source, abstracting away the hard dependency on the CSV file schema.
    - [x] Decouple `TimeSeriesData` struct from specific columns like `prices` and `volumes` to support dynamic feature sets.

## 🏗 Architectural Refactoring
- [x] **Decoupled Indexing Flow:** Create a separate flow/utility for indexing. This process will read the entire historical dataset, create the FAISS index, and persist the created index to disk (e.g., as a `.index` file) along with necessary metadata.
- [x] **Decoupled Query Flow:** Create a separate flow/utility for querying. This process will load the persisted index from disk and perform fast lookups without needing to re-index the historical data.
- [ ] **Optimize Persistent Storage:** Evaluate and implement optimizations for storing the index and metadata, potentially migrating from local `.index` files to a local database (e.g., SQLite, DuckDB) or a dedicated vector database for better scalability and management.
- [ ] **Index Versioning:** Store metadata (WINDOW_SIZE, normalization type) alongside the persisted index to ensure consistency during lookups.
- [] **Change `engine.cpp`: (Low hanging fruit)** This contain common util used by IndexBackend and hence should be renamed.

## 🚀 Service & API Layer
- [x] **Service & API (MVP Critical):** Expose indexing and querying capabilities as a production-ready service. *Crucial because this is the primary monetization surface for B2B clients, Hedge Funds, and automated bot developers (SaaS model).*
    - [x] **Library Packaging:** Refactor the core engine into a clean shared library (`.so` / `.dylib`) for easier integration into broader financial systems and the API layer.
    - [x] **C++ Service Core:** Develop a concurrency wrapper around the shared library to handle concurrent search requests.
    - [x] **REST API:** Create a robust REST API layer (e.g., Python/FastAPI wrapper or Crow C++) that exposes the service endpoints.

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
- [ ] **Visualization Dashboard (MVP Critical):** Build a simple web interface (React/Next.js) to display query patterns and their nearest matches side-by-side using charts. *Crucial because it serves as the secondary monetization surface for retail traders and visually demonstrates the API's power to prospective B2B buyers.*
- [ ] **Chatbot Interface (B2C Focus):** Create integrations with messaging platforms (WhatsApp, Discord, Telegram) to allow users to subscribe to specific pattern alerts or query recent patterns directly from their phones.
- [ ] **Live Data Sources:** Implement connectors for real-time financial data APIs (e.g., Alpha Vantage, Polygon.io). *(Depends on: Optimize Persistent Storage)*

## 📄 Documentation
- [ ] **API Documentation:** Generate Doxygen or similar documentation for the C++ core.
- [ ] **Architectural Overview:** Document the decoupled indexing/querying flow for future maintainers.
