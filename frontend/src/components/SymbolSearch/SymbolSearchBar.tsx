'use client';
// ============================================================
// src/components/SymbolSearch/SymbolSearchBar.tsx
// Controlled symbol input with a popular-symbols dropdown.
// ============================================================

import { useState, useRef, useEffect } from 'react';
import { Search, ChevronDown, X } from 'lucide-react';
import { POPULAR_SYMBOLS } from '@/lib/constants';
import styles from './SymbolSearchBar.module.css';

interface Props {
  value: string;
  onChange: (value: string) => void;
  onSubmit: (symbol: string) => void;
  isLoading?: boolean;
}

export function SymbolSearchBar({ value, onChange, onSubmit, isLoading = false }: Props) {
  const [open, setOpen] = useState(false);
  const ref = useRef<HTMLDivElement>(null);

  // Close dropdown on outside click
  useEffect(() => {
    function handleClickOutside(e: MouseEvent) {
      if (ref.current && !ref.current.contains(e.target as Node)) {
        setOpen(false);
      }
    }
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  function handleKeyDown(e: React.KeyboardEvent<HTMLInputElement>) {
    if (e.key === 'Enter' && value.trim()) {
      setOpen(false);
      onSubmit(value.trim().toUpperCase());
    }
    if (e.key === 'Escape') setOpen(false);
  }

  function handleSelect(sym: string) {
    onChange(sym);
    setOpen(false);
    onSubmit(sym);
  }

  return (
    <div ref={ref} className={styles.wrapper}>
      <div className={styles.inputRow}>
        <Search className={styles.searchIcon} size={18} />
        <input
          id="symbol-input"
          type="text"
          className={styles.input}
          placeholder="Enter symbol — e.g. RELIANCE.NS, AAPL, ^NSEI"
          value={value}
          onChange={(e) => onChange(e.target.value.toUpperCase())}
          onKeyDown={handleKeyDown}
          onFocus={() => setOpen(true)}
          autoComplete="off"
          spellCheck={false}
        />
        {value && (
          <button className={styles.clearBtn} onClick={() => { onChange(''); setOpen(false); }} aria-label="Clear">
            <X size={14} />
          </button>
        )}
        <button
          className={styles.toggleBtn}
          onClick={() => setOpen((o) => !o)}
          aria-label="Popular symbols"
        >
          <ChevronDown size={14} className={open ? styles.chevronOpen : ''} />
        </button>
        <button
          id="search-btn"
          className={styles.searchBtn}
          onClick={() => value.trim() && onSubmit(value.trim().toUpperCase())}
          disabled={!value.trim() || isLoading}
        >
          {isLoading ? <span className={styles.spinner} /> : 'Search'}
        </button>
      </div>

      {open && (
        <ul className={styles.dropdown}>
          <li className={styles.dropdownLabel}>Popular Symbols</li>
          {POPULAR_SYMBOLS.map((s) => (
            <li key={s.value}>
              <button className={styles.dropdownItem} onClick={() => handleSelect(s.value)}>
                <span className={styles.dropdownSymbol}>{s.value}</span>
                <span className={styles.dropdownName}>{s.label}</span>
              </button>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}
