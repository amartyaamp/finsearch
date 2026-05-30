'use client';
// ============================================================
// src/components/MatchCard/MatchCard.tsx
// Displays a single historical pattern match result with
// confidence score, date range, and outcome annotation.
// ============================================================

import { TrendingUp, TrendingDown, Minus, Calendar, Target } from 'lucide-react';
import type { SearchMatch } from '@/lib/api';
import { confidenceColour, formatDate } from '@/lib/chartUtils';
import { MATCH_COLOURS } from '@/lib/chartUtils';
import styles from './MatchCard.module.css';

interface Props {
  match: SearchMatch;
  rank: number;
  isSelected: boolean;
  onClick: () => void;
}

export function MatchCard({ match, rank, isSelected, onClick }: Props) {
  const colour = MATCH_COLOURS[Math.min(rank - 1, MATCH_COLOURS.length - 1)];
  const confColour = confidenceColour(match.confidence_score);
  const hasOutcome = match.outcome_pct !== undefined && match.outcome_pct !== null;
  const outcomePositive = hasOutcome && (match.outcome_pct ?? 0) >= 0;

  return (
    <button
      id={`match-card-${rank}`}
      className={`${styles.card} ${isSelected ? styles.selected : ''}`}
      onClick={onClick}
      style={{ '--accent': colour } as React.CSSProperties}
    >
      {/* Rank badge */}
      <div className={styles.rankBadge} style={{ borderColor: colour, color: colour }}>
        #{rank}
      </div>

      {/* Confidence ring */}
      <div className={styles.scoreSection}>
        <ConfidenceRing score={match.confidence_score} colour={confColour} />
        <div className={styles.scoreLabel}>Confidence</div>
      </div>

      {/* Date range */}
      <div className={styles.infoSection}>
        <div className={styles.infoRow}>
          <Calendar size={12} className={styles.infoIcon} />
          <span className={styles.infoText}>
            {formatDate(match.start_date)} → {formatDate(match.end_date)}
          </span>
        </div>
        <div className={styles.infoRow}>
          <Target size={12} className={styles.infoIcon} />
          <span className={styles.infoText}>
            Distance: {match.distance.toFixed(3)}
          </span>
        </div>
      </div>

      {/* Outcome annotation — the killer feature */}
      {hasOutcome ? (
        <div className={`${styles.outcome} ${outcomePositive ? styles.outcomePos : styles.outcomeNeg}`}>
          {outcomePositive
            ? <TrendingUp size={14} />
            : <TrendingDown size={14} />}
          <span className={styles.outcomePct}>
            {outcomePositive ? '+' : ''}{(match.outcome_pct ?? 0).toFixed(2)}%
          </span>
          <span className={styles.outcomeLabel}>after pattern</span>
        </div>
      ) : (
        <div className={`${styles.outcome} ${styles.outcomeNeutral}`}>
          <Minus size={12} />
          <span className={styles.outcomeLabel}>Outcome loading…</span>
        </div>
      )}
    </button>
  );
}

// ─── Confidence Ring (SVG arc) ────────────────────────────

function ConfidenceRing({ score, colour }: { score: number; colour: string }) {
  const radius = 28;
  const circumference = 2 * Math.PI * radius;
  const offset = circumference - (score / 100) * circumference;

  return (
    <div className={styles.ring}>
      <svg width="72" height="72" viewBox="0 0 72 72">
        {/* Track */}
        <circle
          cx="36" cy="36" r={radius}
          fill="none" stroke="#1f2d47" strokeWidth="5"
        />
        {/* Progress */}
        <circle
          cx="36" cy="36" r={radius}
          fill="none" stroke={colour} strokeWidth="5"
          strokeDasharray={circumference}
          strokeDashoffset={offset}
          strokeLinecap="round"
          transform="rotate(-90 36 36)"
          style={{ transition: 'stroke-dashoffset 0.8s ease' }}
        />
      </svg>
      <span className={styles.ringValue} style={{ color: colour }}>
        {score.toFixed(0)}%
      </span>
    </div>
  );
}
