## Financial Pattern Search (FAISS Implementation)

This project provides a high-performance similarity search tool for financial time series data (e.g., stock prices). It uses **FAISS** for vector indexing and **Z-score normalization** to match patterns based on shape rather than absolute price levels.

### Key Features
- **Confidence Scoring**: Normalized similarity scores (0-100%) for intuitive results.
- **Metadata Tagging**: Supports tracking timestamps/dates for matched patterns.
- **Decoupled Workflow**: Separate commands for building an index and searching against it.

---

### Prerequisites
- **Docker**: Recommended for a consistent build environment (includes FAISS, CMake 3.28+, and GTest).

---

### 1. Build the Project (Docker)

To build the environment and compile the source code:

```bash
docker build -t fin_search .
```

This image contains the compiled binaries: `fin_search` (main CLI) and `fin_search_test` (unit tests).

---

### 2. Using the CLI

The application uses a command-based interface.

#### **A. Build an Index**
To process a historical CSV and save a reusable FAISS index:

```bash
docker run --rm -v "$(pwd)":/app fin_search \
  ./fin_search index data/nifty_50.csv my_index.faiss
```
- `data/nifty_50.csv`: Path to historical data (inside `/app`).
- `my_index.faiss`: Output filename for the index.

#### **B. Search for Patterns**
To search for the most recent pattern (last 60 rows) in a query file against an existing index:

```bash
docker run --rm -v "$(pwd)":/app fin_search \
  ./fin_search search my_index.faiss data/nifty_50.csv
```
The results will include **Confidence Scores** and **Date Ranges** for the matches.

---

### 3. Running Unit Tests

To verify the implementation logic:

```bash
docker run --rm fin_search ./fin_search_test
```

---

### 4. Local Build (Advanced)

If you have FAISS and CMake 3.24+ installed locally:

```bash
mkdir build && cd build
cmake ..
make
./fin_search --help
```

---

### Data Format
The CSV should contain a `Close` price column (Z-score normalization is applied automatically) and metadata columns for dates/timestamps.
