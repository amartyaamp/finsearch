'use client';
// ============================================================
// src/components/ErrorBanner/ErrorBanner.tsx
// Dismissable error message shown when API calls fail.
// ============================================================

import { AlertTriangle, X } from 'lucide-react';
import styles from './ErrorBanner.module.css';

interface Props {
  message: string;
  onDismiss: () => void;
}

export function ErrorBanner({ message, onDismiss }: Props) {
  return (
    <div className={styles.banner} role="alert">
      <AlertTriangle size={16} className={styles.icon} />
      <span className={styles.message}>{message}</span>
      <button className={styles.dismiss} onClick={onDismiss} aria-label="Dismiss error">
        <X size={14} />
      </button>
    </div>
  );
}
