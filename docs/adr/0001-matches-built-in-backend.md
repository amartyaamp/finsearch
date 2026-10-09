# Matches are built in the backend, from price history stored with the index

Matches used to carry only a date range, so each client rebuilt them from the *searched* instrument's prices — the wrong instrument, and empty for matches older than the client's one-year fetch. We decided the backend returns complete matches (instrument, bars, outcome), assembled from price history saved alongside the index at index time, because a second client (the planned Telegram bot) would otherwise have to duplicate that logic, and serving from stored price history keeps search offline and consistent with what was indexed.

## Considered Options

- **Keep match building in the frontend** — every client re-implements it; the bug lives in each copy.
- **Store bars inside the C++ index and return them through the C API** — moves arrays across the C seam with manual memory ownership; similarity stays C++'s only job instead.
- **Fetch match bars live from yfinance per match** — one network call per match, throttling, and data that can differ from what was indexed.
- **DuckDB now** — planned (TODO.md P1), but widens this fix. Price history is a flat file behind a small price store interface, so the DuckDB migration replaces only what's behind it.

## Consequences

- The index format changes (instrument per window, versioned metadata); existing index files are regenerated, not migrated.
- Search fails with 503 when price history is missing rather than returning partial matches.
