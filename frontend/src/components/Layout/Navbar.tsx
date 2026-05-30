'use client';
// ============================================================
// src/components/Layout/Navbar.tsx
// Top navigation bar with branding and a live status indicator
// for the FastAPI backend connection.
// ============================================================

import { useState, useEffect } from 'react';
import { Activity, Github, Zap } from 'lucide-react';
import { apiClient } from '@/lib/api';
import styles from './Navbar.module.css';

export function Navbar() {
  const [apiStatus, setApiStatus] = useState<'checking' | 'online' | 'offline'>('checking');

  useEffect(() => {
    async function checkApi() {
      try {
        await apiClient.get('/docs', { timeout: 3000 });
        setApiStatus('online');
      } catch {
        // FastAPI /docs might not return 200 via proxy, try /ohlcv
        try {
          await apiClient.get('/ohlcv', { params: { symbol: 'TEST' }, timeout: 3000 });
          setApiStatus('online');
        } catch {
          setApiStatus('offline');
        }
      }
    }
    checkApi();
    const id = setInterval(checkApi, 30_000);
    return () => clearInterval(id);
  }, []);

  return (
    <nav className={styles.nav}>
      <div className={styles.inner}>
        {/* Brand */}
        <a href="/" className={styles.brand}>
          <span className={styles.brandIcon}><Zap size={18} fill="currentColor" /></span>
          <span className={styles.brandName}>FinSearch</span>
          <span className={styles.brandTag}>Pattern Engine</span>
        </a>

        {/* Right side */}
        <div className={styles.right}>
          {/* API status dot */}
          <div className={styles.statusPill} title={`Backend API: ${apiStatus}`}>
            <Activity size={12} />
            <span
              className={`${styles.statusDot} ${
                apiStatus === 'online' ? styles.dotOnline :
                apiStatus === 'offline' ? styles.dotOffline :
                styles.dotChecking
              }`}
            />
            <span className={styles.statusLabel}>
              {apiStatus === 'checking' ? 'Connecting…' : apiStatus === 'online' ? 'Live' : 'Offline'}
            </span>
          </div>

          <a
            href="https://github.com"
            target="_blank"
            rel="noopener noreferrer"
            className={styles.githubLink}
            aria-label="GitHub"
          >
            <Github size={18} />
          </a>
        </div>
      </div>
    </nav>
  );
}
