# FinSearch

Finds moments in market history whose price shape resembles an instrument's recent price action, and shows what happened next.

## Market data

**Instrument**:
A tradeable symbol whose price history can be searched or indexed, e.g. `^NSEI`, `RELIANCE.NS`.
_Avoid_: Ticker, stock, asset

**Bar**:
One period of an instrument's trading — date, open, high, low, close, volume.
_Avoid_: Candle, candlestick, row, OHLCV row

**Feature**:
A per-bar series that similarity is measured on, such as close or volume.
_Avoid_: Column, dimension, signal

**Price history**:
The bars of every instrument an index was built from, kept alongside that index.
_Avoid_: Raw data, CSV, OHLCV data

## Searching

**Window**:
A run of consecutive bars of one instrument, of fixed length (the window size).
_Avoid_: Pattern (when meaning the stored span), segment, slice

**Index**:
The searchable collection of windows taken from the price history of one or more instruments.
_Avoid_: Database, FAISS file

**Query window**:
The most recent window of the instrument being searched.
_Avoid_: Query pattern, current pattern, query features

**Match**:
A window in the index that resembles the query window, identified by its instrument and date range.
_Avoid_: Result, hit, neighbour

**Confidence score**:
A 0–100% measure of how closely a match resembles the query window.
_Avoid_: Similarity, accuracy, probability

**Outcome**:
The percentage price change over a fixed number of bars after a match ends — from the open of the first bar after the match to the close of the last bar in that span.
_Avoid_: Return, performance, result

**Outcome horizon**:
The number of bars after a match that its outcome covers.
_Avoid_: Outcome days, lookahead

**Self-match**:
A match that is the query window itself, or overlaps it on the same instrument.
