# Math Notes

This document summarises the key formulas and assumptions used in the library.

---

## 1) Market model and notation

- Spot: \(S_0\)
- Strike: \(K\)
- Maturity (time to expiry): \(T\) (in years)
- Risk-free rate: \(r\) (continuously compounded)
- Continuous dividend yield: \(q\)
- Volatility: \(\sigma\)

Discount factors:
\[
D_r = e^{-rT}, \qquad D_q = e^{-qT}
\]

Under the Black–Scholes model with continuous dividend yield, under the risk-neutral measure:
\[
\frac{dS_t}{S_t} = (r - q)\,dt + \sigma\,dW_t
\]

---

## 2) Black–Scholes European prices

Define:
\[
d_1 = \frac{\ln(S_0/K) + (r - q + \tfrac{1}{2}\sigma^2)T}{\sigma\sqrt{T}},
\qquad
d_2 = d_1 - \sigma\sqrt{T}
\]
Let \(N(\cdot)\) be the standard normal CDF and \(n(\cdot)\) the standard normal PDF.

### Call
\[
C = S_0 D_q N(d_1) - K D_r N(d_2)
\]

### Put
\[
P = K D_r N(-d_2) - S_0 D_q N(-d_1)
\]

---

## 3) Put–call parity

For European options:
\[
C - P = S_0 D_q - K D_r
\]

---

## 4) No-arbitrage bounds (used in implied vol solver)

### Call bounds
\[
\max(0, S_0 D_q - K D_r) \le C \le S_0 D_q
\]

### Put bounds
\[
\max(0, K D_r - S_0 D_q) \le P \le K D_r
\]

If a quoted market price violates these bounds, there is **no implied volatility** under Black–Scholes.

---

## 5) Black–Scholes Greeks

Let \(n(d_1)\) be the standard normal pdf evaluated at \(d_1\).

### Delta
\[
\Delta_C = D_q N(d_1),
\qquad
\Delta_P = D_q (N(d_1) - 1)
\]

### Gamma (call and put)
\[
\Gamma = \frac{D_q n(d_1)}{S_0 \sigma \sqrt{T}}
\]

### Vega (per 1.0 volatility change, call and put)
\[
\text{Vega} = S_0 D_q n(d_1)\sqrt{T}
\]

### Rho (per 1.0 rate change)
\[
\rho_C = K T D_r N(d_2),
\qquad
\rho_P = -K T D_r N(-d_2)
\]

### Theta convention
This project uses **calendar theta**:
\[
\Theta = \frac{\partial V}{\partial t}
\]
Since \(T\) is time-to-maturity and \(T = T_{\text{maturity}} - t\), we have:
\[
\frac{\partial V}{\partial t} = -\frac{\partial V}{\partial T}
\]

The standard calendar theta formulas are:

Call:
\[
\Theta_C = -\frac{S_0 D_q n(d_1)\sigma}{2\sqrt{T}}
- rK D_r N(d_2) + q S_0 D_q N(d_1)
\]

Put:
\[
\Theta_P = -\frac{S_0 D_q n(d_1)\sigma}{2\sqrt{T}}
+ rK D_r N(-d_2) - q S_0 D_q N(-d_1)
\]

---

## 6) CRR binomial tree (European and American)

Let the number of steps be \(N\), and \(\Delta t = T/N\).

CRR parameters:
\[
u = e^{\sigma\sqrt{\Delta t}},\qquad d = \frac{1}{u}
\]
Risk-neutral probability with dividend yield:
\[
p = \frac{e^{(r-q)\Delta t} - d}{u - d},\qquad 1-p
\]
Per-step discount factor:
\[
\text{disc} = e^{-r\Delta t}
\]

At node \((n,i)\) (step \(n\), \(i\) up moves), the stock price is:
\[
S_{n,i} = S_0 u^i d^{n-i}
\]

### European option by backward induction
Terminal payoff at maturity:
\[
V_{N,i} = \max(0, \pm(S_{N,i}-K))
\]
Rollback:
\[
V_{n,i} = e^{-r\Delta t}\left(p V_{n+1,i+1} + (1-p)V_{n+1,i}\right)
\]

### American option by backward induction + early exercise
At each node:
\[
V_{n,i} = \max\left(\text{payoff}(S_{n,i}),\ e^{-r\Delta t}\left(p V_{n+1,i+1} + (1-p)V_{n+1,i}\right)\right)
\]

---

## 7) Implied volatility (European, BS)

We solve for \(\sigma\) in:
\[
f(\sigma) = BS(\sigma) - P_{\text{mkt}} = 0
\]

For European vanilla options, Black–Scholes price is increasing in \(\sigma\) (Vega > 0), so bisection is reliable when the solution is bracketed.

Bracketing strategy:
- Start with \(\sigma_{\text{lo}}\) near 0 and \(\sigma_{\text{hi}}\) moderate (e.g. 2.0)
- Increase \(\sigma_{\text{hi}}\) until \(BS(\sigma_{\text{hi}})\ge P_{\text{mkt}}\)
- Bisection until price error or sigma interval is below tolerance