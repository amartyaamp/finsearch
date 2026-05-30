// ============================================================
// src/lib/api.ts
// Centralised API client — all backend calls live here.
// Change NEXT_PUBLIC_API_BASE in .env.local to point at a
// different environment without touching any component.
// ============================================================

import axios from 'axios';

const BASE_URL = process.env.NEXT_PUBLIC_API_BASE ?? '/api/backend';

export const apiClient = axios.create({
  baseURL: BASE_URL,
  timeout: 30_000,
  headers: { 'Content-Type': 'application/json' },
});

// ─── Types ────────────────────────────────────────────────

export interface OHLCVBar {
  time: string;   // "YYYY-MM-DD"
  open: number;
  high: number;
  low: number;
  close: number;
  volume: number;
}

export interface SearchMatch {
  distance: number;
  confidence_score: number;
  index: number;
  start_date: string;
  end_date: string;
  // enriched on the client side after fetching outcome data:
  outcome_pct?: number | null;
  outcome_candles?: OHLCVBar[];
}

export interface SearchResponse {
  status: string;
  results: SearchMatch[];
}

// ─── API Functions ────────────────────────────────────────

/**
 * Fetch OHLCV candlestick data for a symbol over a date range.
 * Omit fromDate / toDate to get the last year of data.
 */
export async function fetchOHLCV(
  symbol: string,
  fromDate?: string,
  toDate?: string,
): Promise<OHLCVBar[]> {
  const params: Record<string, string> = { symbol };
  if (fromDate) params.from_date = fromDate;
  if (toDate) params.to_date = toDate;

  const { data } = await apiClient.get<OHLCVBar[]>('/ohlcv', { params });
  return data;
}

/**
 * Run a pattern search for a symbol.
 * Returns matched historical windows sorted by confidence.
 */
export async function searchSymbol(
  symbol: string,
  windowSize: number = 60,
  kNeighbors: number = 5,
): Promise<SearchMatch[]> {
  const { data } = await apiClient.post<SearchResponse>('/search/symbol', {
    symbol,
    window_size: windowSize,
    k_neighbors: kNeighbors,
  });
  return data.results;
}

/**
 * Enrich a list of matches by fetching the outcome bars
 * (the N candles immediately after each match's end_date)
 * and computing the percentage price change.
 */
export async function enrichMatchesWithOutcomes(
  symbol: string,
  matches: SearchMatch[],
  outcomeDays: number = 20,
): Promise<SearchMatch[]> {
  return Promise.all(
    matches.map(async (match) => {
      try {
        // Fetch bars starting from the day after match ends
        const afterStart = offsetDate(match.end_date, 1);
        const afterEnd = offsetDate(match.end_date, outcomeDays + 5);
        const bars = await fetchOHLCV(symbol, afterStart, afterEnd);
        const slicedBars = bars.slice(0, outcomeDays);

        if (slicedBars.length === 0) return { ...match, outcome_pct: null, outcome_candles: [] };

        const entryClose = slicedBars[0].open;
        const exitClose = slicedBars[slicedBars.length - 1].close;
        const outcome_pct = ((exitClose - entryClose) / entryClose) * 100;

        return { ...match, outcome_pct, outcome_candles: slicedBars };
      } catch {
        return { ...match, outcome_pct: null, outcome_candles: [] };
      }
    }),
  );
}

// ─── Helpers ─────────────────────────────────────────────

function offsetDate(dateStr: string, days: number): string {
  const d = new Date(dateStr);
  d.setDate(d.getDate() + days);
  return d.toISOString().split('T')[0];
}
