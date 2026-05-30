'use client';
// ============================================================
// src/app/page.tsx
// Root page — composes all feature components into the
// FinSearch dashboard. All data-fetching logic lives here;
// child components stay purely presentational.
// ============================================================

import { useState, useCallback, useRef } from 'react';
import { BarChart2, GitBranch } from 'lucide-react';

import { SymbolSearchBar } from '@/components/SymbolSearch/SymbolSearchBar';
import { SearchControls } from '@/components/SymbolSearch/SearchControls';
import { CandlestickChart } from '@/components/Chart/CandlestickChart';
import { PatternOverlayChart } from '@/components/Chart/PatternOverlayChart';
import { MatchCardList } from '@/components/MatchCard/MatchCardList';
import { ErrorBanner } from '@/components/ErrorBanner/ErrorBanner';
import { Navbar } from '@/components/Layout/Navbar';

import {
  fetchOHLCV,
  searchSymbol,
  enrichMatchesWithOutcomes,
  type OHLCVBar,
  type SearchMatch,
} from '@/lib/api';
import { DEFAULT_WINDOW_SIZE, DEFAULT_K_NEIGHBORS, DEFAULT_OUTCOME_DAYS } from '@/lib/constants';
import styles from './page.module.css';

type TabId = 'candlestick' | 'overlay';

export default function HomePage() {
  // ── Form state ────────────────────────────────────────
  const [symbol, setSymbol] = useState('');
  const [windowSize, setWindowSize] = useState(DEFAULT_WINDOW_SIZE);
  const [kNeighbors, setKNeighbors] = useState(DEFAULT_K_NEIGHBORS);
  const [outcomeDays, setOutcomeDays] = useState(DEFAULT_OUTCOME_DAYS);

  // ── Data state ────────────────────────────────────────
  const [ohlcv, setOhlcv] = useState<OHLCVBar[]>([]);
  const [matches, setMatches] = useState<SearchMatch[]>([]);
  const [activeSymbol, setActiveSymbol] = useState<string>('');

  // ── UI state ──────────────────────────────────────────
  const [isSearching, setIsSearching] = useState(false);
  const [isFetchingOutcomes, setIsFetchingOutcomes] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [activeTab, setActiveTab] = useState<TabId>('candlestick');
  const [selectedMatchIdx, setSelectedMatchIdx] = useState<number | null>(null);

  const resultsRef = useRef<HTMLDivElement>(null);

  // ── Search handler ────────────────────────────────────
  const handleSearch = useCallback(async (sym: string) => {
    if (!sym) return;
    setError(null);
    setMatches([]);
    setSelectedMatchIdx(null);
    setIsSearching(true);
    setActiveSymbol(sym);

    try {
      // Fetch OHLCV and search in parallel
      const [ohlcvData, searchResults] = await Promise.all([
        fetchOHLCV(sym),
        searchSymbol(sym, windowSize, kNeighbors),
      ]);

      setOhlcv(ohlcvData);
      setMatches(searchResults);

      // Scroll to results
      setTimeout(() => resultsRef.current?.scrollIntoView({ behavior: 'smooth', block: 'start' }), 100);

      // Enrich matches with outcome data asynchronously (non-blocking)
      setIsFetchingOutcomes(true);
      enrichMatchesWithOutcomes(sym, searchResults, outcomeDays)
        .then((enriched) => setMatches(enriched))
        .finally(() => setIsFetchingOutcomes(false));
    } catch (err: unknown) {
      const msg = err instanceof Error ? err.message : 'Unknown error';
      setError(
        `Failed to search "${sym}": ${msg}. ` +
          'Make sure the FastAPI backend is running on port 8000 and the index is loaded.'
      );
    } finally {
      setIsSearching(false);
    }
  }, [windowSize, kNeighbors, outcomeDays]);

  // ── Derived data ──────────────────────────────────────
  const queryBars = ohlcv.slice(-windowSize);

  return (
    <>
      <Navbar />

      <main className={styles.main}>
        {/* ── Hero section ── */}
        <section className={styles.hero}>
          <div className={styles.heroBadge}>
            <span className={styles.heroBadgeDot} />
            FAISS-powered · Z-score similarity · Real-time Yahoo Finance data
          </div>
          <h1 className={styles.heroTitle}>
            Find historical patterns that&nbsp;
            <span className={styles.heroAccent}>match today</span>
          </h1>
          <p className={styles.heroSubtitle}>
            Enter any NSE, BSE, or global ticker symbol to find the top historical windows
            with the same price shape — and see what happened next.
          </p>

          {/* ── Search bar ── */}
          <SymbolSearchBar
            value={symbol}
            onChange={setSymbol}
            onSubmit={handleSearch}
            isLoading={isSearching}
          />

          {/* ── Parameter controls ── */}
          <SearchControls
            windowSize={windowSize}
            kNeighbors={kNeighbors}
            outcomeDays={outcomeDays}
            onWindowSizeChange={setWindowSize}
            onKNeighborsChange={setKNeighbors}
            onOutcomeDaysChange={setOutcomeDays}
          />
        </section>

        {/* ── Error ── */}
        {error && (
          <div className={styles.errorWrapper}>
            <ErrorBanner message={error} onDismiss={() => setError(null)} />
          </div>
        )}

        {/* ── Results ── */}
        {(ohlcv.length > 0 || isSearching) && (
          <section ref={resultsRef} className={styles.results}>
            {/* Section header */}
            {activeSymbol && (
              <div className={styles.sectionHeader}>
                <h2 className={styles.sectionTitle}>
                  <span className={styles.sectionSymbol}>{activeSymbol}</span>
                  {isFetchingOutcomes && (
                    <span className={styles.outcomesBadge}>Enriching outcomes…</span>
                  )}
                </h2>
                <p className={styles.sectionSubtitle}>
                  Showing {matches.length} historical match{matches.length !== 1 ? 'es' : ''} for the last {windowSize}-day pattern
                </p>
              </div>
            )}

            {/* ── Chart area ── */}
            {ohlcv.length > 0 && (
              <div className={styles.chartSection}>
                {/* Tab switcher */}
                <div className={styles.tabs}>
                  <button
                    id="tab-candlestick"
                    className={`${styles.tab} ${activeTab === 'candlestick' ? styles.tabActive : ''}`}
                    onClick={() => setActiveTab('candlestick')}
                  >
                    <BarChart2 size={14} />
                    Candlestick (1yr)
                  </button>
                  {matches.length > 0 && (
                    <button
                      id="tab-overlay"
                      className={`${styles.tab} ${activeTab === 'overlay' ? styles.tabActive : ''}`}
                      onClick={() => setActiveTab('overlay')}
                    >
                      <GitBranch size={14} />
                      Pattern Overlay
                    </button>
                  )}
                </div>

                {activeTab === 'candlestick' && (
                  <CandlestickChart bars={ohlcv} height={360} />
                )}
                {activeTab === 'overlay' && matches.length > 0 && (
                  <PatternOverlayChart
                    queryBars={queryBars}
                    matches={matches}
                    allOhlcv={ohlcv}
                    windowSize={windowSize}
                    height={360}
                  />
                )}
              </div>
            )}

            {/* ── Match cards ── */}
            <div className={styles.matchSection}>
              <h3 className={styles.matchTitle}>Historical Pattern Matches</h3>
              <MatchCardList
                matches={matches}
                isLoading={isSearching}
                selectedIndex={selectedMatchIdx}
                onSelect={setSelectedMatchIdx}
              />
            </div>
          </section>
        )}

        {/* ── Empty state hero stats ── */}
        {ohlcv.length === 0 && !isSearching && !error && (
          <section className={styles.statsRow}>
            <StatBox label="Similarity Engine" value="FAISS L2" sub="Facebook AI Research" />
            <StatBox label="Normalisation" value="Z-Score" sub="Shape-invariant matching" />
            <StatBox label="Data Source" value="Yahoo Finance" sub="NSE · BSE · US markets" />
            <StatBox label="Latency" value="< 200ms" sub="C++ search core" />
          </section>
        )}
      </main>
    </>
  );
}

function StatBox({ label, value, sub }: { label: string; value: string; sub: string }) {
  return (
    <div className={styles.statBox}>
      <p className={styles.statLabel}>{label}</p>
      <p className={styles.statValue}>{value}</p>
      <p className={styles.statSub}>{sub}</p>
    </div>
  );
}
