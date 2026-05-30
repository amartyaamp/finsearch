'use client';
// ============================================================
// src/components/Chart/PatternOverlayChart.tsx
// Overlays the query pattern against top-3 historical matches
// as normalised line series on a single chart.
// ============================================================

import { useEffect, useRef } from 'react';
import type { OHLCVBar, SearchMatch } from '@/lib/api';
import { THEME } from '@/lib/constants';
import { normalizeBarsToBase, barsToLineSeries, getTopMatches, MATCH_COLOURS } from '@/lib/chartUtils';
import styles from './CandlestickChart.module.css';

interface Props {
  queryBars: OHLCVBar[];
  matches: SearchMatch[];
  allOhlcv: OHLCVBar[];
  windowSize: number;
  height?: number;
}

export function PatternOverlayChart({ queryBars, matches, allOhlcv, windowSize, height = 320 }: Props) {
  const containerRef = useRef<HTMLDivElement>(null);
  const chartRef = useRef<unknown>(null);

  useEffect(() => {
    if (!containerRef.current || queryBars.length === 0) return;

    let isMounted = true;

    (async () => {
      const { createChart } = await import('lightweight-charts');
      if (!isMounted || !containerRef.current) return;

      if (chartRef.current) {
        (chartRef.current as { remove: () => void }).remove();
      }

      const chart = createChart(containerRef.current, {
        width: containerRef.current.clientWidth,
        height,
        layout: {
          background: { color: THEME.chart.bg },
          textColor: THEME.chart.text,
          fontFamily: "'JetBrains Mono', monospace",
        },
        grid: {
          vertLines: { color: THEME.chart.grid },
          horzLines: { color: THEME.chart.grid },
        },
        rightPriceScale: { borderColor: THEME.chart.grid },
        timeScale: { borderColor: THEME.chart.grid, timeVisible: true },
        leftPriceScale: { visible: false },
      });

      chartRef.current = chart;

      const typedChart = chart as {
        addLineSeries: (opts: unknown) => { setData: (d: unknown) => void };
        timeScale: () => { fitContent: () => void };
      };

      // ── Query pattern (current) ─────────────────────────
      const queryNorm = normalizeBarsToBase(queryBars, 100);
      const querySeries = typedChart.addLineSeries({
        color: '#6366f1',
        lineWidth: 2,
        title: 'Current',
        priceLineVisible: false,
        lastValueVisible: false,
      });
      querySeries.setData(barsToLineSeries(queryNorm) as unknown[]);

      // ── Historical matches (top 3) ────────────────────
      const top = getTopMatches(matches, 3);

      top.forEach((match, i) => {
        const colour = MATCH_COLOURS[i];
        const matchBars = allOhlcv
          .filter((b) => b.time >= match.start_date && b.time <= match.end_date)
          .slice(0, windowSize);

        if (matchBars.length === 0) return;

        // Normalise to same base and re-index to query timeframe for overlap
        const normalised = normalizeBarsToBase(matchBars, 100);
        const queryTimestamps = barsToLineSeries(queryNorm).map((d) => d.time);

        const remapped = normalised.map((bar, idx) => ({
          time: queryTimestamps[idx] ?? bar.time,
          value: bar.close,
        }));

        const matchSeries = typedChart.addLineSeries({
          color: colour,
          lineWidth: 1,
          lineStyle: 2, // dashed
          title: `Match ${i + 1} (${match.start_date})`,
          priceLineVisible: false,
          lastValueVisible: false,
        });

        matchSeries.setData(remapped.filter((d) => d.time) as unknown[]);
      });

      typedChart.timeScale().fitContent();

      const ro = new ResizeObserver(() => {
        if (containerRef.current && chartRef.current) {
          (chartRef.current as { applyOptions: (o: unknown) => void }).applyOptions({
            width: containerRef.current.clientWidth,
          });
        }
      });
      ro.observe(containerRef.current);
      return () => ro.disconnect();
    })();

    return () => {
      isMounted = false;
      if (chartRef.current) {
        (chartRef.current as { remove: () => void }).remove();
        chartRef.current = null;
      }
    };
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [queryBars, matches, allOhlcv, windowSize, height]);

  return (
    <div className={styles.wrapper}>
      <div className={styles.legend}>
        <span className={styles.legendItem} style={{ '--colour': '#6366f1' } as React.CSSProperties}>
          <span className={styles.legendDot} /> Current Pattern
        </span>
        {getTopMatches(matches, 3).map((m, i) => (
          <span key={m.index} className={styles.legendItem} style={{ '--colour': MATCH_COLOURS[i] } as React.CSSProperties}>
            <span className={styles.legendDash} /> Match {i + 1} · {m.start_date}
          </span>
        ))}
      </div>
      <div ref={containerRef} style={{ height }} />
    </div>
  );
}
