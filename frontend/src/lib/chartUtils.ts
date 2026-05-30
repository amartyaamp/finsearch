// ============================================================
// src/lib/chartUtils.ts
// Pure utility functions for TradingView Lightweight Charts.
// No React deps — safe to import in non-component contexts.
// ============================================================

import type { OHLCVBar, SearchMatch } from './api';

/**
 * Converts a hex colour to an rgba string.
 */
export function hexToRgba(hex: string, alpha: number): string {
  const r = parseInt(hex.slice(1, 3), 16);
  const g = parseInt(hex.slice(3, 5), 16);
  const b = parseInt(hex.slice(5, 7), 16);
  return `rgba(${r}, ${g}, ${b}, ${alpha})`;
}

/**
 * Normalises an array of OHLCV bars to start at a base value
 * (useful when overlaying historical matches on a chart so they
 * share the same visual starting point as the query pattern).
 */
export function normalizeBarsToBase(bars: OHLCVBar[], baseValue: number = 100): OHLCVBar[] {
  if (bars.length === 0) return [];
  const firstClose = bars[0].close;
  if (firstClose === 0) return bars;
  const ratio = baseValue / firstClose;
  return bars.map((bar) => ({
    ...bar,
    open: bar.open * ratio,
    high: bar.high * ratio,
    low: bar.low * ratio,
    close: bar.close * ratio,
  }));
}

/**
 * Converts OHLCV bars to a simple line series (close prices).
 */
export function barsToLineSeries(bars: OHLCVBar[]): { time: string; value: number }[] {
  return bars.map((b) => ({ time: b.time, value: b.close }));
}

/**
 * Colours used for the top-3 match overlays on the chart.
 */
export const MATCH_COLOURS = ['#f59e0b', '#10b981', '#a78bfa'] as const;

/**
 * Given a confidence score (0-100), returns a colour interpolated
 * from red → amber → green.
 */
export function confidenceColour(score: number): string {
  if (score >= 75) return '#10b981'; // emerald
  if (score >= 50) return '#f59e0b'; // amber
  return '#ef4444';                   // red
}

/**
 * Formats a date string to human-readable "DD MMM YYYY".
 */
export function formatDate(dateStr: string): string {
  try {
    return new Date(dateStr).toLocaleDateString('en-IN', {
      day: '2-digit',
      month: 'short',
      year: 'numeric',
    });
  } catch {
    return dateStr;
  }
}

/**
 * Given a list of SearchMatch objects, extracts the top N by
 * confidence_score for chart overlay display.
 */
export function getTopMatches(matches: SearchMatch[], n: number = 3): SearchMatch[] {
  return [...matches]
    .sort((a, b) => b.confidence_score - a.confidence_score)
    .slice(0, n);
}
