'use client';
// ============================================================
// src/components/Chart/CandlestickChart.tsx
// Renders a OHLCV candlestick chart using TradingView
// Lightweight Charts (runs only on the client).
// ============================================================

import { useEffect, useRef } from 'react';
import type { OHLCVBar } from '@/lib/api';
import { THEME } from '@/lib/constants';
import styles from './CandlestickChart.module.css';

interface Props {
  bars: OHLCVBar[];
  height?: number;
  highlightRange?: { from: string; to: string };
}

export function CandlestickChart({ bars, height = 340, highlightRange }: Props) {
  const containerRef = useRef<HTMLDivElement>(null);
  const chartRef = useRef<unknown>(null);
  const seriesRef = useRef<unknown>(null);

  useEffect(() => {
    if (!containerRef.current || bars.length === 0) return;

    let chart: unknown;
    let isMounted = true;

    (async () => {
      const { createChart, CrosshairMode } = await import('lightweight-charts');
      if (!isMounted || !containerRef.current) return;

      // Destroy previous chart instance
      if (chartRef.current) {
        (chartRef.current as { remove: () => void }).remove();
      }

      chart = createChart(containerRef.current, {
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
        crosshair: {
          mode: CrosshairMode.Normal,
          vertLine: { color: THEME.chart.crosshair, width: 1, style: 1 },
          horzLine: { color: THEME.chart.crosshair, width: 1, style: 1 },
        },
        rightPriceScale: {
          borderColor: THEME.chart.grid,
          textColor: THEME.chart.text,
        },
        timeScale: {
          borderColor: THEME.chart.grid,
          timeVisible: true,
        },
      });

      chartRef.current = chart;

      const typedChart = chart as {
        addCandlestickSeries: (opts: unknown) => { setData: (d: unknown) => void; createPriceLine?: (opts: unknown) => void };
        timeScale: () => { fitContent: () => void };
      };

      const series = typedChart.addCandlestickSeries({
        upColor: THEME.chart.up,
        downColor: THEME.chart.down,
        borderUpColor: THEME.chart.up,
        borderDownColor: THEME.chart.down,
        wickUpColor: THEME.chart.up,
        wickDownColor: THEME.chart.down,
      });

      seriesRef.current = series;

      const sortedBars = [...bars].sort((a, b) => a.time.localeCompare(b.time));
      series.setData(sortedBars as unknown[]);
      typedChart.timeScale().fitContent();

      // Resize observer
      const ro = new ResizeObserver(() => {
        if (containerRef.current && chartRef.current) {
          (chartRef.current as { applyOptions: (opts: unknown) => void }).applyOptions({
            width: containerRef.current.clientWidth,
          });
        }
      });
      ro.observe(containerRef.current);

      return () => ro.disconnect();
    })();

    return () => {
      isMounted = false;
      if (chart) {
        (chart as { remove: () => void }).remove();
        chartRef.current = null;
      }
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [bars, height]);

  return (
    <div className={styles.wrapper}>
      <div ref={containerRef} className={styles.chart} style={{ height }} />
    </div>
  );
}
