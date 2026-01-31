# Overview

This repository is a lightweight C++ options pricing library focused on **clarity**, **correctness**, and **testability**.

It implements:
- **European vanilla options** (call/put)
  - Black–Scholes closed-form analytic pricer (with continuous dividend yield `q`)
  - Cox–Ross–Rubinstein (CRR) binomial tree pricer (validated against Black–Scholes)
- **American vanilla options** (call/put)
  - CRR binomial tree pricer with early exercise
- **Greeks**
  - Black–Scholes analytic Greeks (Delta, Gamma, Vega, Theta, Rho)
  - Greeks verified against finite-difference derivatives of the Black–Scholes price
- **Implied volatility (European only)**
  - Robust bisection solver for Black–Scholes implied vol with no-arbitrage bounds + bracketing

A small CLI (`optcli`) is provided to price options, display Greeks, and solve implied volatility.

---

### This project is:
- A clean reference implementation of standard computational finance building blocks
- A test-driven codebase that encodes economic properties (bounds, parity, monotonicity)
- A base you can extend with PDE methods, calibration, or more payoffs

---

## Supported instruments

- Vanilla European call/put
- Vanilla American call/put

Assumptions:
- Continuous dividend yield `q`
- Risk-free rate `r` treated as constant
- Black–Scholes volatility `sigma` treated as constant

---

## Validation philosophy

The repo includes unit tests that check:
- **Put–call parity** (Black–Scholes)
- **No-arbitrage bounds**
- **Monotonicity** (e.g., call price increases with `S0`, decreases with `K`)
- **Tree convergence** toward Black–Scholes for European options
- **American inequalities** (American $\geq$ European; call early exercise behaviour when `q=0`)
- **Implied vol** recovers known `sigma` and throws on bound violations
- **Greeks** match finite differences of the analytic price

See:
- `docs/NUMERICS_AND_TESTING.md`

---

## Project layout

- `include/` – public headers
- `src/pricers/` – pricing engines (BS analytic, CRR tree, implied vol)
- `src/main.cpp` – CLI entry point
- `tests/` – unit tests (single test runner)
- `docs/` – documentation (this folder)

See:
- `docs/DESIGN.md`