# FinSearch — Roadmap

Product direction and staging. Individual work items live in [GitHub Issues](https://github.com/amartyaamp/finsearch/issues), labelled `P1` (next sprint), `P2` (polish & scale) and `future`. Domain vocabulary is in [`CONTEXT.md`](../CONTEXT.md); decisions are in [`docs/adr/`](adr/).

## Who it's for

- **Retail traders (India)** — the dashboard is the primary product surface. The outcome annotation (what price did in the N bars after a matched pattern) is the feature traders will pay for.
- **Algo traders and quant devs** — the search API, gated by API keys and rate limits.
- **B2B buyers** (brokerages, fintech platforms) — a live demo first, then a white-label API or embedded widget.

## Staging

| Stage | Target | Stack | Distribution |
|---|---|---|---|
| **v0.1 — Demo** | Developers / curious traders | Railway + pre-indexed Nifty data | README badge, Twitter/X, Reddit |
| **v0.2 — Product** | Retail traders (India) | Next.js dashboard + Telegram bot | r/IndiaInvestments, Zerodha forums |
| **v1.0 — API SaaS** | Algo traders, quant devs | API keys + rate limiting + DuckDB | RapidAPI marketplace |
| **v2.0 — B2B** | Brokerages, fintech platforms | White-label API / embedded widget | Direct outreach to Zerodha / Dhan / Upstox |

## Current state

v0.1 is deployed on Railway with five years of Nifty 50 and Bank Nifty pre-indexed. Next: complete matches (#1), so the outcome annotation is correct before wider sharing — then share on r/IndiaInvestments and the Zerodha TradingView forums for the first 100 real users.

## Gates

- **Before sharing a live URL publicly:** API key auth (#6).
- **Before charging B2B clients:** API keys + rate limiting (#6).
- **Before multi-instrument nightly indexing:** DuckDB storage (#7) and index versioning (#8).
