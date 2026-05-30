'use client';
// ============================================================
// src/components/MatchCard/MatchCardList.tsx
// Renders the full grid of MatchCards, with a loading skeleton.
// ============================================================

import type { SearchMatch } from '@/lib/api';
import { MatchCard } from './MatchCard';
import styles from './MatchCardList.module.css';

interface Props {
  matches: SearchMatch[];
  isLoading: boolean;
  selectedIndex: number | null;
  onSelect: (index: number) => void;
}

export function MatchCardList({ matches, isLoading, selectedIndex, onSelect }: Props) {
  if (isLoading) {
    return (
      <div className={styles.grid}>
        {Array.from({ length: 5 }).map((_, i) => (
          <div key={i} className={styles.skeleton} />
        ))}
      </div>
    );
  }

  if (matches.length === 0) {
    return (
      <div className={styles.empty}>
        <span className={styles.emptyIcon}>🔍</span>
        <p>No results yet. Enter a symbol and press Search to find matching patterns.</p>
      </div>
    );
  }

  return (
    <div className={styles.grid}>
      {matches.map((match, i) => (
        <MatchCard
          key={match.index}
          match={match}
          rank={i + 1}
          isSelected={selectedIndex === i}
          onClick={() => onSelect(i)}
        />
      ))}
    </div>
  );
}
