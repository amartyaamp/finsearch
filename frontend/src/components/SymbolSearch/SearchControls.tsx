'use client';
// ============================================================
// src/components/SymbolSearch/SearchControls.tsx
// Window-size & k-neighbors sliders, shown below the search bar.
// ============================================================

import styles from './SearchControls.module.css';

interface Props {
  windowSize: number;
  kNeighbors: number;
  outcomeDays: number;
  onWindowSizeChange: (v: number) => void;
  onKNeighborsChange: (v: number) => void;
  onOutcomeDaysChange: (v: number) => void;
}

export function SearchControls({
  windowSize,
  kNeighbors,
  outcomeDays,
  onWindowSizeChange,
  onKNeighborsChange,
  onOutcomeDaysChange,
}: Props) {
  return (
    <div className={styles.wrapper}>
      <SliderField
        id="window-size-slider"
        label="Pattern Window"
        value={windowSize}
        min={20}
        max={100}
        step={10}
        unit="days"
        note="Length of the query pattern extracted from the most recent bars."
        onChange={onWindowSizeChange}
      />
      <SliderField
        id="k-neighbors-slider"
        label="Top Matches"
        value={kNeighbors}
        min={1}
        max={10}
        step={1}
        unit="results"
        note="Number of historical pattern matches to return."
        onChange={onKNeighborsChange}
      />
      <SliderField
        id="outcome-days-slider"
        label="Outcome Window"
        value={outcomeDays}
        min={5}
        max={60}
        step={5}
        unit="days"
        note="Days after each matched pattern used to calculate price outcome."
        onChange={onOutcomeDaysChange}
      />
    </div>
  );
}

interface SliderFieldProps {
  id: string;
  label: string;
  value: number;
  min: number;
  max: number;
  step: number;
  unit: string;
  note: string;
  onChange: (v: number) => void;
}

function SliderField({ id, label, value, min, max, step, unit, note, onChange }: SliderFieldProps) {
  const pct = ((value - min) / (max - min)) * 100;
  return (
    <div className={styles.field}>
      <div className={styles.fieldHeader}>
        <label htmlFor={id} className={styles.fieldLabel}>{label}</label>
        <span className={styles.fieldValue}>{value} <span className={styles.fieldUnit}>{unit}</span></span>
      </div>
      <div className={styles.sliderWrapper}>
        <input
          id={id}
          type="range"
          min={min}
          max={max}
          step={step}
          value={value}
          onChange={(e) => onChange(Number(e.target.value))}
          className={styles.slider}
          style={{ '--pct': `${pct}%` } as React.CSSProperties}
        />
      </div>
      <p className={styles.fieldNote}>{note}</p>
    </div>
  );
}
