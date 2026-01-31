// AnalyticBS.cpp: Implementation of Black-Scholes European Analytic Pricer
#include "pricers/AnalyticBS.hpp"
#include "util/Math.hpp"
#include <stdexcept> 
#include <cmath> 

namespace pricers {
    void AnalyticBS::check_inputs(const opt::Market& m, const opt::Option& o) {
        if (m.S0 <= 0.0) throw std::invalid_argument("Spot Price must be positive.");
        if (o.K <= 0.0) throw std::invalid_argument("Strike Price must be positive.");
        if (o.T <= 0.0) throw std::invalid_argument("Time to Maturity must be positive.");
        if (m.sigma <= 0.0) throw std::invalid_argument("Volatility must be positive.");
        if (o.exercise != opt::Exercise::European) throw std::invalid_argument("Analytic Black-Scholes only support European Options.");
    }

    static inline void d1d2(const opt::Market& m, const opt::Option& o, double& d1, double& d2) {
        const double T = o.T; 
        const double sig = m.sigma;
        const double sqrtT = std::sqrt(T);
        const double volSqrtT = sig * sqrtT;

        const double lnSK = std::log(m.S0 / o.K);
        d1 = (lnSK + (m.r - m.q + 0.5 * sig * sig) * T) / volSqrtT;
        d2 = d1  - volSqrtT;
    }

    double AnalyticBS::price(const opt::Market& m, const opt::Option& o) {
        check_inputs(m, o);

        double d1, d2;
        d1d2(m, o, d1, d2);

        const double discFactorR = std::exp(-m.r * o.T);
        const double discFactorQ = std::exp(-m.q * o.T);

        if (o.type == opt::OptionType::Call) {
            return m.S0 * discFactorQ * util::normal_cdf(d1) - o.K * discFactorR * util::normal_cdf(d2);
        } else {
            return o.K * discFactorR * util::normal_cdf(-d2) - m.S0 * discFactorQ * util::normal_cdf(-d1);
        }
    }

    Greeks AnalyticBS::greeks(const opt::Market& m, const opt::Option& o) {
        check_inputs(m, o);

        double d1, d2;
        d1d2(m, o, d1, d2);

        const double T = o.T;
        const double sqrtT = std::sqrt(T);
        const double discFactorR = std::exp(-m.r * T);
        const double discFactorQ = std::exp(-m.q * T);

        const double Nd1 = util::normal_cdf(d1);
        const double Nd2 = util::normal_cdf(d2);
        const double nD1 = util::normal_pdf(d1);

        Greeks g; 

        // Delta 
        if (o.type == opt::OptionType::Call) g.delta = discFactorQ * Nd1;
        else g.delta = discFactorQ * (Nd1 - 1.0);

        // Gamma (Same for Call and Put)
        g.gamma = (discFactorQ * nD1) / (m.S0 * m.sigma * sqrtT);

        // Vega (Same for Call and Put) (per unit vol not percentage)
        g.vega = m.S0 * discFactorQ * nD1 * sqrtT;

        // Rho (per 1.0 rate change not percentage)
        if (o.type == opt::OptionType::Call) g.rho = o.K * T * discFactorR * Nd2;
        else g.rho = -o.K * T * discFactorR * util::normal_cdf(-d2);

        // Theta (per year)
        const double term1 = -(m.S0 * discFactorQ * nD1 * m.sigma) / (2.0 * sqrtT);
        if (o.type == opt::OptionType::Call) {
            g.theta = term1 - m.r * o.K * discFactorR * Nd2 + m.q * m.S0 * discFactorQ * Nd1;
        } else {
            g.theta = term1 + m.r * o.K * discFactorR * util::normal_cdf(-d2) - m.q * m.S0 * discFactorQ * util::normal_cdf(-d1);
        }

        return g;
    }

} // namespace pricers
