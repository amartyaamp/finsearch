# FinSearch — Frontend Dashboard

A Next.js 14 visualization dashboard for the FinSearch pattern search engine.

## Project Structure

```
frontend/
├── next.config.js              # API proxy rewrite to FastAPI :8000
├── package.json
├── tsconfig.json
├── .env.local                  # NEXT_PUBLIC_API_BASE=/api/backend
└── src/
    ├── app/
    │   ├── layout.tsx          # Root layout — fonts, SEO metadata
    │   ├── page.tsx            # Main dashboard page (data orchestration)
    │   ├── page.module.css     # Page-level layout styles
    │   └── globals.css         # CSS reset + design tokens
    ├── components/
    │   ├── Layout/
    │   │   ├── Navbar.tsx              # Sticky nav + live API status pill
    │   │   └── Navbar.module.css
    │   ├── SymbolSearch/
    │   │   ├── SymbolSearchBar.tsx     # Symbol input + popular-symbols dropdown
    │   │   ├── SymbolSearchBar.module.css
    │   │   ├── SearchControls.tsx      # Window/k-neighbors/outcome sliders
    │   │   └── SearchControls.module.css
    │   ├── Chart/
    │   │   ├── CandlestickChart.tsx    # TradingView Lightweight Charts — OHLCV
    │   │   ├── PatternOverlayChart.tsx # Normalised pattern overlay (query vs matches)
    │   │   └── CandlestickChart.module.css
    │   ├── MatchCard/
    │   │   ├── MatchCard.tsx           # Single match: confidence ring + outcome badge
    │   │   ├── MatchCard.module.css
    │   │   ├── MatchCardList.tsx       # Grid of cards with skeleton loader
    │   │   └── MatchCardList.module.css
    │   └── ErrorBanner/
    │       ├── ErrorBanner.tsx         # Dismissable API error banner
    │       └── ErrorBanner.module.css
    └── lib/
        ├── api.ts              # All backend API calls + TypeScript types
        ├── chartUtils.ts       # Pure chart utilities (normalisation, colours)
        └── constants.ts        # Symbols list, defaults, theme tokens
```

## Getting Started

### Prerequisites

- Node.js 18+
- FastAPI backend running on `http://localhost:8000` with an index loaded

### Install & Run

```bash
cd frontend
npm install
npm run dev
```

Open [http://localhost:3000](http://localhost:3000).

### Environment

```env
# .env.local — already created
NEXT_PUBLIC_API_BASE=/api/backend
```

The `next.config.js` proxies `/api/backend/*` → `http://localhost:8000/*`, so you never hit CORS issues in development.

For production, set `NEXT_PUBLIC_API_BASE` to your deployed FastAPI URL.

## Key Design Decisions

| Decision | Rationale |
|---|---|
| **CSS Modules** | Scoped styles with zero runtime — no Tailwind dependency |
| **Dynamic import for Lightweight Charts** | TradingView library is browser-only; dynamic import avoids SSR crash |
| **Outcome enrichment is non-blocking** | OHLCV + search results display immediately; outcome data enriches cards progressively |
| **`page.tsx` owns all state** | Components are purely presentational — easy to test and swap |
| **Next.js API proxy** | Avoids CORS headers on FastAPI in dev; single URL change for prod |
