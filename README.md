# FinSearch — Financial Pattern Search Engine

A high-performance similarity search engine for financial time series data. FinSearch uses **FAISS** vector indexing and **Z-score normalization** to find historical patterns that match the *shape* of a query window, regardless of absolute price level.

---

## Architecture Overview

```
┌───────────────────────────────────────────────────────┐
│                    REST API Layer                     │
│          FastAPI (Python) — api/main.py               │
│   Routes: POST /index, POST /search                   │
└───────────────────────┬───────────────────────────────┘
                        │ ctypes FFI
┌───────────────────────▼───────────────────────────────┐
│               C++ Service Core (libfin_search_core)   │
│  service_core  ←→  engine  ←→  faiss_index_backend    │
│  (thread-safe)     (normalize)    (FAISS L2 index)    │
└───────────────────────┬───────────────────────────────┘
                        │
┌───────────────────────▼───────────────────────────────┐
│                  CLI (fin_search)                     │
│          index <csv> <out.faiss>                      │
│          search <index.faiss> <query.csv>             │
└───────────────────────────────────────────────────────┘
```

### Source Layout

```
finsearch/
├── src/
│   ├── main.cpp                # CLI entry point (index / search commands)
│   ├── engine.cpp / .h         # Z-score normalisation + window slicing
│   ├── faiss_index_backend.cpp/.h  # FAISS FlatL2 index wrapper
│   ├── index_backend.h         # Abstract IndexBackend interface
│   ├── service_core.cpp / .h   # Thread-safe C++ service wrapper
│   ├── c_api.cpp / .h          # C ABI exposed for Python FFI
│   ├── csv_parser.cpp / .h     # CSV row parsing
│   ├── csv_data_source.cpp/.h  # DataSource impl reading from CSV
│   └── data_source.h           # Abstract DataSource interface
├── api/
│   ├── main.py                 # FastAPI application (2 endpoints)
│   ├── fin_search_bind.py      # Python ctypes bindings to libfin_search_core
│   ├── requirements.txt        # fastapi, uvicorn, pytest, httpx
│   └── tests/
│       └── test_api.py         # Python API integration tests
├── tests/
│   ├── engine_test.cpp
│   ├── faiss_index_backend_test.cpp
│   ├── csv_parser_test.cpp
│   ├── csv_data_source_test.cpp
│   └── service_core_test.cpp
├── data/
│   ├── nifty_50.csv            # ~51 MB historical OHLCV data
│   └── sample.csv              # Small query sample
├── scripts/
│   └── run_local_ci.sh         # One-shot local build + test script
├── CMakeLists.txt
└── Dockerfile                  # Ubuntu 22.04 + FAISS from source + GTest
```

---

## Prerequisites

| Requirement | Details |
|---|---|
| **Docker** | Recommended. Bundles FAISS, CMake 3.28, GTest, Python 3, pip |
| **Local build** | CMake ≥ 3.10, FAISS (C++ native), OpenBLAS, GTest, Python 3.9+ |

---

## Quick Start — Local CI (Recommended)

Runs a full build + C++ unit tests + Python API tests in Docker, then auto-cleans up the image and container:

```bash
chmod +x scripts/run_local_ci.sh
./scripts/run_local_ci.sh
```

---

## Docker Usage

### 1. Build the Image

```bash
docker build -t fin_search .
```

The image compiles `fin_search` (CLI), `libfin_search_core` (shared library), and `fin_search_test` (test binary), then installs Python dependencies.

### 2. Run Unit Tests

The default Docker `CMD` runs both C++ and Python API tests:

```bash
docker run --rm fin_search
```

### 3. Build a FAISS Index from CSV

Processes a historical CSV and writes `<name>.faiss` + `<name>.faiss.meta` to the mounted directory:

```bash
docker run --rm -v "$(pwd)":/app fin_search \
  fin_search index data/nifty_50.csv my_index.faiss
```

> **Note:** Both `.faiss` and `.faiss.meta` must be present for searching to work.

### 4. Search Against an Index

Extracts the last 60 rows from the query CSV and returns the top-K most similar historical windows:

```bash
docker run --rm -v "$(pwd)":/app fin_search \
  fin_search search my_index.faiss data/sample.csv
```

Results include **Confidence Score** (0–100%) and **Date Range** for each match.

### 5. Run the REST API

```bash
docker run -p 8000:8000 --rm -v "$(pwd)":/app fin_search \
  bash -c "uvicorn api.main:app --host 0.0.0.0 --port 8000"
```

Interactive API docs available at **[http://localhost:8000/docs](http://localhost:8000/docs)**

---

## REST API Reference

| Method | Endpoint | Description |
|---|---|---|
| `POST` | `/index` | Build a FAISS index from a CSV file path |
| `POST` | `/search` | Search the in-memory index with a float feature matrix |

### `POST /index`
```json
{
  "csv_file_path": "/app/data/nifty_50.csv",
  "window_size": 60
}
```

### `POST /search`
```json
{
  "query_features": [[...60 floats...], [...60 floats...]],
  "k_neighbors": 5,
  "window_size": 60
}
```

**Response:**
```json
{
  "status": "success",
  "results": [
    {
      "distance": 1.23,
      "confidence_score": 87.5,
      "index": 412,
      "start_date": "2021-03-01",
      "end_date": "2021-05-21"
    }
  ]
}
```

---

## Local Build (Advanced)

If you have FAISS (C++ native), CMake ≥ 3.10, OpenBLAS, and GTest installed:

```bash
mkdir build && cd build
cmake ..
make
```

Run the CLI:
```bash
./fin_search index ../data/nifty_50.csv my_index.faiss
./fin_search search my_index.faiss ../data/sample.csv
```

Run C++ tests:
```bash
./fin_search_test
```

Run Python API (from repo root):
```bash
pip install -r api/requirements.txt
uvicorn api.main:app --reload
```

---

## Data Format

CSV files must contain a `Close` column (used for Z-score normalization). Additional columns (e.g., `Date`, `Volume`) are preserved as metadata and returned alongside search results.

**Example:**
```
Date,Close,Volume
2020-01-01,11800.50,150000000
2020-01-02,11850.00,162000000
...
```

---

## CI/CD

GitHub Actions runs on every push:
- Builds the Docker image
- Executes `fin_search_test` (C++ GTest suite)
- Executes `pytest api/tests/test_api.py` (Python API tests)

---

## What's Next

See [TODO.md](./TODO.md) for the full roadmap. Top priorities:

- **Visualisation dashboard** — Next.js chart overlay UI (TradingView Lightweight Charts)
- **Live data connector** — `yfinance` integration for symbol-based querying (no CSV upload needed)
- **Public deployment** — Railway one-command deploy with pre-indexed Nifty 50 data
- **API key auth** — Rate-limited key issuance for B2B monetisation
- **DuckDB storage** — Replace flat `.faiss`/`.meta` files with a versioned database
