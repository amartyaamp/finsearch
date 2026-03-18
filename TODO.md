# FinSearch — Project Roadmap & TODO

## ✅ Done — Core Engine

- [x] **Multivariate support (MVP):** Pattern matching across multiple time series (e.g., Price + Volume). Confirms institutional participation context.
  - [ ] **Future:** Weighted feature distances — assign different importance to Price vs Volume during similarity calculation.
- [x] **Metadata tagging (MVP):** Store and return timestamps/dates and contextual metadata alongside index positions.
- [x] **Confidence scoring (MVP):** Map absolute vector distances to a normalised similarity score (0–100%). Intuitive for retail traders and external systems.
- [x] **Arbitrary data sources:** Decouple `TimeSeriesData` struct from fixed columns (`prices`, `volumes`) to support dynamic feature sets.
- [x] **Decoupled indexing flow:** Separate CLI command reads historical data, builds FAISS index, and persists `.faiss` + `.meta` files to disk.
- [x] **Decoupled query flow:** Separate CLI command loads persisted index from disk and performs fast lookups without re-indexing.
- [x] **Library packaging:** Refactored core engine into a clean shared library (`.so` / `.dylib`) for API integration.
- [x] **C++ concurrency wrapper:** Handles concurrent search requests around the shared library.
- [x] **REST API (FastAPI):** Production-ready endpoints for indexing and querying, exposed via Python FFI wrapper.
- [x] **CI/CD integration:** GitHub Actions pipeline runs tests and builds Docker images on every push.
- [x] **Docker environment:** Consistent build environment with FAISS, CMake 3.28+, and GTest bundled.

---

## 🔴 P0 — Ship Now (Frontend + Live Data)

These are the last items between you and a publicly shareable product.

- [ ] **Human-friendly search API (`POST /search/symbol`):**
  The current `POST /search` requires callers to supply raw `query_features` — a multi-dimensional float matrix that no human or UI can construct manually. Replace this with a symbol-based endpoint that accepts a ticker string and does the data fetching internally.
  - [ ] Accept `{ "symbol": "RELIANCE.NS", "window_size": 60, "k_neighbors": 5 }`
  - [ ] Fetch OHLCV via `yfinance.download()` and extract the last `window_size` rows
  - [ ] Build the float matrix internally and delegate to the existing C++ search core
  - [ ] Add a companion `GET /ohlcv?symbol=X&from=Y&to=Z` endpoint so the frontend can fetch historical candles for chart overlay rendering

- [ ] **Input validation layer:**
  Validate query patterns and historical data ranges at the API boundary before they reach the C++ core. Must ship before any public demo deployment.

- [ ] **Visualisation dashboard (MVP critical):**
  Build a Next.js web interface as the primary product surface for retail traders and a live demo for B2B buyers.
  - [ ] Symbol picker or CSV upload input
  - [ ] Pattern search trigger with loading state
  - [ ] Chart overlay view: current pattern vs top 3 historical matches (use TradingView Lightweight Charts — free, purpose-built for finance)
  - [ ] Confidence score badge + matched date range displayed per result
  - [ ] **Outcome annotation per match:** Show what price did in the N candles *after* each matched pattern ended (e.g., +7.2% / −3.1% over next 10 days). This is the feature traders will pay for.

- [ ] **Live data connector — Yahoo Finance (MVP critical):**
  Wire `yfinance` as a thin Python wrapper feeding into the existing indexer. No API key required, covers NSE/BSE/global symbols. This eliminates the CSV-upload friction for end users. Scoped to fetch-only per request (no caching) for v0.1.
  - [ ] Auto-fetch OHLCV on symbol entry in the dashboard
  - [ ] **Future:** Alpha Vantage connector (paid tier for intraday)
  - [ ] **Future:** Polygon.io connector (US markets, real-time)

- [ ] **Public demo deployment:**
  Deploy to Railway (Docker-native, `railway up` one-command deploy). Pre-index 5 years of Nifty 50 + Bank Nifty data. Add a "Try it live" badge to the README. Share on r/IndiaInvestments and Zerodha Tradingview forums for first 100 real users.

---

## 🟡 P1 — Next Sprint (Productionisation)

- [ ] **API key auth layer:**
  Add API key generation and validation before sharing any live URL publicly. Even a simple `.env`-backed key list is fine for v1. This is the monetisation gate — without it you cannot charge B2B clients.
  - [ ] Key issuance endpoint (`POST /api/keys`)
  - [ ] Middleware validation on all search/index routes
  - [ ] Rate limiting per key (e.g., 100 searches/day on free tier)

- [ ] **DuckDB persistent storage:**
  Migrate from flat `.faiss` + `.meta` files to a DuckDB database. Enables multi-symbol index management, versioning, query history, and usage analytics in a single zero-infra file.
  - [ ] Schema: `symbols`, `indexes`, `search_history`
  - [ ] Migration script from current `.faiss` / `.meta` format
  - *(Unblocks: yfinance response caching, multi-symbol batch indexing)*

- [ ] **Index versioning:**
  Store `WINDOW_SIZE`, normalization type, data source hash, and creation timestamp alongside each persisted index. Prevents silent wrong results when an index is rebuilt with different parameters.

- [ ] **Multi-symbol batch indexing:**
  Add a YAML config listing symbols (e.g., `RELIANCE.NS`, `HDFCBANK.NS`, `NIFTY50`) + a cron job that auto-rebuilds indexes nightly from live data. Makes the product feel live rather than manual.

- [ ] **Configurable CLI parameters:**
  Pass `WINDOW_SIZE` and `K_NEIGHBORS` as command-line arguments instead of compile-time constants. Low-hanging fruit — unlocks experimentation without rebuilds.

---

## 🟢 P2 — Polish & Scale

- [ ] **Telegram / WhatsApp pattern alert bot (B2C growth):**
  Users subscribe to a symbol and receive a Telegram message when a historically significant pattern is detected. Telegram Bot API is free and ~2 hours to wire. Viral distribution channel for Indian retail traders.
  - [ ] `/search RELIANCE 60` — returns top match + confidence + chart image
  - [ ] `/subscribe RELIANCE` — daily pattern digest
  - [ ] WhatsApp via Twilio sandbox (secondary, behind Telegram)

- [ ] **Optimised CSV parsing:**
  Handle malformed CSV files, missing values, and varied date formats gracefully (currently fails silently on bad input).

- [ ] **Extended distance metrics:**
  Implement Inner Product and Cosine Similarity in addition to L2. Cosine similarity is often preferred for shape matching on normalised vectors.

- [ ] **Normalization strategies:**
  Add Min-Max scaling and Robust Scaler options alongside existing Z-score normalisation. Robust Scaler is better for data with outliers (e.g., crash days).

- [ ] **Expanded test suite:**
  - [ ] Unit tests for edge cases in `normalize_window` (zero-variance windows, single-point series)
  - [ ] Mock tests for FAISS index creation failure scenarios
  - [ ] Integration tests for the FastAPI endpoints

- [ ] **engine.cpp rename (low-hanging fruit):**
  `engine.cpp` contains common utilities shared by `IndexBackend` and should be renamed to reflect its utility role (e.g., `utils.cpp` or `common.cpp`).

- [ ] **FAISS-GPU support:**
  GPU-accelerated search for significantly larger datasets (10M+ vectors). Evaluate `faiss-gpu` Docker base image.

- [ ] **Memory optimisation:**
  Profile and optimise memory consumption when loading very large historical CSVs (>5 years, tick-level data).

---

## 📄 Documentation

- [ ] **API documentation:** Generate Doxygen documentation for the C++ core library.
- [ ] **Architectural overview:** Document the decoupled indexing/querying flow, the FFI bridge, and the concurrency model for future contributors.
- [ ] **Deployment guide:** Step-by-step Railway / Render deployment with environment variables, pre-indexing script, and health check setup.

---

## 🗺 Deployment Roadmap Summary

| Stage | Target | Stack | Distribution |
|---|---|---|---|
| **v0.1 — Demo** | Developers / curious traders | Railway + pre-indexed Nifty data | README badge, Twitter/X, Reddit |
| **v0.2 — Product** | Retail traders (India) | Next.js dashboard + Telegram bot | r/IndiaInvestments, Zerodha forums |
| **v1.0 — API SaaS** | Algo traders, quant devs | API keys + rate limiting + DuckDB | RapidAPI marketplace |
| **v2.0 — B2B** | Brokerages, fintech platforms | White-label API / embedded widget | Direct outreach to Zerodha / Dhan / Upstox |