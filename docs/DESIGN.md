# Design

This document explains how the codebase is organised and how to extend it.

---

## 1) Folder layout

Typical structure:

- `include/`
  - `opt/` – domain types (Market, Option, enums)
  - `pricers/` – pricing engines (BS analytic, CRR tree, implied vol)
  - `util/` – utilities (normal CDF/PDF, small math helpers)
- `src/`
  - `pricers/` – implementations for pricers
  - `main.cpp` – CLI entry point
- `tests/` – unit tests and minimal test framework
- `docs/` – documentation

This layout keeps public interfaces in `include/` and implementations in `src/`.

---

## 2) Core domain types

### Market (`opt::Market`)
Contains:
- `S0` spot
- `r` risk-free rate
- `q` dividend yield
- `sigma` volatility

### Option (`opt::Option`)
Contains:
- `K` strike
- `T` maturity (years)
- `type` (Call/Put)
- `exercise` (European/American)

The pricers assume vanilla payoffs only.

---

## 3) Pricing engines

### A) Black–Scholes analytic pricer
File(s):
- `pricers/AnalyticBS.hpp/.cpp`

Responsibilities:
- compute European price using closed-form BS (with q)
- compute analytic Greeks (Delta/Gamma/Vega/Theta/Rho)
- validate inputs (positive spot, positive strike, etc.)

### B) CRR binomial tree pricer
File(s):
- `pricers/BinomialCRR.hpp/.cpp`

Responsibilities:
- compute u, d, p, discount
- European pricing via backward induction (no early exercise)
- American pricing via backward induction with early-exercise max

Implementation detail:
- uses **O(N) memory** by storing only the value vector for the “next” time slice and rolling back in place.
- avoids building an explicit node graph (no pointers, no heap node objects).

### C) Implied volatility (BS, European only)
File(s):
- `pricers/ImpliedVol.hpp/.cpp`

Responsibilities:
- check BS no-arbitrage bounds
- bracket sigma and solve via bisection
- throw on inconsistent market prices

---

## 4) CLI design

File:
- `src/main.cpp`

The CLI follows a simple `--key value` pattern.

Key flags:
- `--style euro|amer`
- `--type call|put`
- `--S0 --K --T --r --q`
- `--sigma` (required unless `--iv`)
- `--N` (tree steps)
- `--greeks` (BS Greeks; European only)
- `--iv --price <target>` (BS implied vol; European only)

The CLI is intentionally lightweight and avoids external parsing libraries.

---

## 5) Tests

- Exactly one file defines the test runner `main()` (e.g. `tests/test_main.cpp`).
- All other test files contain `TEST(...)` blocks only.

Testing philosophy:
- Validate financial identities and inequalities (parity, bounds, monotonicity)
- Validate convergence behaviour (tree → BS)
- Validate numerical derivatives (Greeks via finite differences)

See:
- `docs/NUMERICS_AND_TESTING.md`

---

