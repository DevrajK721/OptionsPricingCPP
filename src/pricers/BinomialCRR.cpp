// BinomialCRR.cpp: Binomial Cox-Ross-Rubinstein (CRR) option pricing model
#include "pricers/BinomialCRR.hpp"
#include <cmath> 
#include <stdexcept>
#include <vector> 
#include <algorithm>

namespace pricers {

    double BinomialCRR::price_european(const opt::Market& m,
                                    const opt::Option& opt,
                                    const TreeParams& p) {
        
        // Check inputs 
        check_inputs(m, opt, p);
        
        // Compute coefficients 
        CRRCoefs coefs = make_coefs(m, opt, p);

        if (opt.exercise != opt::Exercise::European) throw std::invalid_argument("Binomial European Pricer only supports European Options.");
        if (coefs.pu < -1e-12 || coefs.pu > 1.0 + 1e-12) throw std::invalid_argument("Risk-Neutral Probability out of bounds [0,1], please check inputs (increasing N usually helps).");    

        // Clamp 
        if (coefs.pu < 0.0) coefs.pu = 0.0;
        if (coefs.pu > 1.0) coefs.pu = 1.0;
        coefs.pd = 1.0 - coefs.pu;

        // Initialize values 
        const int N = p.steps;
        std::vector<double> values(N + 1);

        double S = m.S0 * std::pow(coefs.d, N); // Price at node (N,0)
        const double u_over_d = coefs.u / coefs.d;

        for (int i = 0; i <= N; ++i) {
            values[i] = payoff(S, opt);
            S *= u_over_d;
        }

        // Solve via backward induction
        for (int step = p.steps - 1; step >= 0; --step) {
            for (int i = 0; i <= step; ++i) {
                values[i] = coefs.disc * (coefs.pu * values[i + 1] + coefs.pd * values[i]);
            }
        }
        
        return values[0];
    }

    double BinomialCRR::price_american(const opt::Market& m,
                                    const opt::Option& opt,
                                    const TreeParams& p) {
        // Check inputs 
        check_inputs(m, opt, p);

        // Compute coefficients
        CRRCoefs coefs = make_coefs(m, opt, p);

        if (opt.exercise != opt::Exercise::American) throw std::invalid_argument("Binomial American Pricer only supports American Options.");
        if (coefs.pu < -1e-12 || coefs.pu > 1.0 + 1e-12) throw std::invalid_argument("Risk-Neutral Probability out of bounds [0,1], please check inputs (increasing N usually helps).");

        // Clamp
        if (coefs.pu < 0.0) coefs.pu = 0.0;
        if (coefs.pu > 1.0) coefs.pu = 1.0;
        coefs.pd = 1.0 - coefs.pu;

        // Initialize values 
        const int N = p.steps;
        std::vector<double> values(N + 1);

        double S = m.S0 * std::pow(coefs.d, N); // Price at node (N, 0)
        const double u_over_d = coefs.u / coefs.d;

        for (int i = 0; i <= N; ++i) {
            values[i] = payoff(S, opt);
            S *= u_over_d;
        }

        // Solve via backward induction with early exercise
        for (int step = p.steps - 1; step >= 0; --step) {
            double Snode = m.S0 * std::pow(coefs.d, step); // Price at node (step, 0)
            for (int i = 0; i <= step; ++i) {
                double exercise_value = payoff(Snode, opt);
                double hold_value = coefs.disc * (coefs.pu * values[i + 1] + coefs.pd * values[i]);
                values[i] = std::max(exercise_value, hold_value);
                Snode *= u_over_d;
            }
        }

        return values[0];
    }

    void BinomialCRR::check_inputs(const opt::Market& m,
                                const opt::Option& opt,
                                const TreeParams& p) {
        if (m.S0 <= 0.0) throw std::invalid_argument("Spot Price must be positive.");
        if (opt.K <= 0.0) throw std::invalid_argument("Strike Price must be positive.");
        if (opt.T <= 0.0) throw std::invalid_argument("Time to Maturity must be positive.");
        if (m.sigma <= 0.0) throw std::invalid_argument("Volatility must be positive.");
        if (p.steps <= 0) throw std::invalid_argument("Number of steps must be positive.");
    }

    BinomialCRR::CRRCoefs BinomialCRR::make_coefs(const opt::Market& m,
                                                const opt::Option& opt,
                                                const TreeParams& p) {
        int N = p.steps;
        double dt = opt.T / N;
        double u = std::exp(m.sigma * std::sqrt(dt));
        double d = 1.0 / u;
        double disc = std::exp(-m.r * dt);

        double pu = (std::exp((m.r - m.q) * dt) - d) / (u - d);
        double pd = 1.0 - pu;

        return {dt, u, d, pu, pd, disc};
    }

    double BinomialCRR::payoff(double S, const opt::Option& opt) {
        // TODO
        if (opt.type == opt::OptionType::Call) {
            return std::max(0.0, S - opt.K);
        } else {
            return std::max(0.0, opt.K - S);
        }
    }

} // namespace pricers