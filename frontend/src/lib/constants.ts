// ============================================================
// src/lib/constants.ts
// App-wide constants — single source of truth for defaults,
// popular symbol lists, and theme tokens.
// ============================================================

export const DEFAULT_WINDOW_SIZE = 60;
export const DEFAULT_K_NEIGHBORS = 5;
export const DEFAULT_OUTCOME_DAYS = 20;
export const API_BASE_URL = process.env.NEXT_PUBLIC_API_BASE ?? '/api/backend';

/** Popular NSE / BSE / US symbols shown in the quick-pick dropdown */
export const POPULAR_SYMBOLS = [
  { label: 'Nifty 50', value: '^NSEI' },
  { label: 'Bank Nifty', value: '^NSEBANK' },
  { label: 'Reliance', value: 'RELIANCE.NS' },
  { label: 'HDFC Bank', value: 'HDFCBANK.NS' },
  { label: 'Infosys', value: 'INFY.NS' },
  { label: 'TCS', value: 'TCS.NS' },
  { label: 'Tata Motors', value: 'TATAMOTORS.NS' },
  { label: 'ICICI Bank', value: 'ICICIBANK.NS' },
  { label: 'Apple', value: 'AAPL' },
  { label: 'Tesla', value: 'TSLA' },
  { label: 'S&P 500', value: 'SPY' },
  { label: 'Nasdaq 100', value: 'QQQ' },
] as const;

/** Theme colour tokens */
export const THEME = {
  bg: '#0a0e1a',
  surface: '#111827',
  surfaceElevated: '#1a2233',
  border: '#1f2d47',
  accent: '#6366f1',  // indigo
  accentGlow: 'rgba(99,102,241,0.25)',
  textPrimary: '#f1f5f9',
  textSecondary: '#94a3b8',
  textMuted: '#475569',
  green: '#10b981',
  red: '#ef4444',
  amber: '#f59e0b',
  purple: '#a78bfa',
  chart: {
    up: '#10b981',
    down: '#ef4444',
    wick: '#475569',
    grid: '#1f2d47',
    bg: '#0d1424',
    crosshair: '#334155',
    text: '#94a3b8',
  },
} as const;
