#include "pricers/ImpliedVol.hpp"
#include "pricers/AnalyticBS.hpp"

#include <cmath> 
#include <stdexcept> 
#include <algorithm>

namespace pricers {
    void ImpliedVol::check_inputs(const opt::Market& m, 
                                 const opt::Option& opt, 
                                 double target_price) {
        if (m.S0 <= 0.0) throw std::invalid_argument("Spot price S0 must be positive.");
        if (opt.K <= 0.0) throw std::invalid_argument("Strike price K must be positive.");
        if (opt.T <= 0.0) throw std::invalid_argument("Time to maturity T must be positive.");
        if (target_price < 0.0) throw std::invalid_argument("Target option price must be non-negative.");

        // Implied Vol for Black-Scholes is only defined for European options
        if (opt.exercise != opt::Exercise::European) {
            throw std::invalid_argument("Implied Volatility can only be computed for European options under Black-Scholes model.");
        }
    }

    void ImpliedVol::bs_bounds(const opt::Market& m, 
                            const opt::Option& opt,
                            double& lower,
                            double& upper) {
        const double T = opt.T;
        const double Dr = std::exp(-m.r * T);
        const double Dq = std::exp(-m.q * T);

        if (opt.type == opt::OptionType::Call) {
            lower = std::max(0.0, m.S0 * Dq - opt.K * Dr);
            upper = m.S0 * Dq;
        } else { 
            lower = std::max(0.0, opt.K * Dr - m.S0 * Dq);
            upper = opt.K * Dr;
        }
    }

    double ImpliedVol::solve_bs(const opt::Market& m_in, 
                              const opt::Option& opt, 
                              double target_price, 
                              const ImpliedVolParams& params) {
        check_inputs(m_in, opt, target_price);
        // Enforce no-arbitrage bounds
        double lb = 0.0, ub = 0.0;
        bs_bounds(m_in, opt, lb, ub);

        const double eps = 1e-12;
        if (target_price < lb - eps || target_price > ub + eps) {
            throw std::invalid_argument("Target price violates no-arbitrage bounds.");
        }

        if (std::fabs(target_price - lb) < params.tol_price) {
            return params.sigma_lo; // Implied vol approaches 0
        }

        // Bracket sigma (volatility)
        double lo = params.sigma_lo;
        double hi = params.sigma_hi;

        opt::Market m = m_in; // Local copy to modify sigma
        auto price_at = [&](double sigma) -> double {
            m.sigma = sigma;
            return AnalyticBS::price(m, opt);
        }; // Lambda expression to compute price at given sigma

        double price_lo = price_at(lo);
        if (price_lo > target_price) {
            return lo; // Implied vol is very low
        }
        double price_hi = price_at(hi);

        // Expand upper bracket until we bracket the target price
        int expand = 0;
        while (price_hi + params.tol_price < target_price && expand < 50) {
            hi *= 2.0;
            price_hi = price_at(hi);
            ++expand;
            if (hi > 10.0) break; // Prevent excessive volatility
        }

        if (price_hi + params.tol_price < target_price) {
            throw std::runtime_error("Failed to bracket target price with volatility.");
        }

        // Bisection Method 
        double mid = 0.0;
        for (int it = 0; it < params.max_iter; ++it) {
            mid = 0.5 * (lo + hi);
            const double pmid = price_at(mid);
            const double err = pmid - target_price;

            if (std::fabs(err) < params.tol_price || (hi - lo) < params.tol_sigma) {
                return mid; // Converged
            }

            // Monotone: if pmid < target_price, need higher sigma
            if (pmid < target_price) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        throw std::runtime_error("Implied volatility solver did not converge within max iterations.");
    }
} // namespace pricers