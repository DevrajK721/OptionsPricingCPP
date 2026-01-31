// AnalyticBS.hpp: Black-Scholes European Analytic Pricer
#pragma once
#include "opt/Market.hpp"
#include "opt/Option.hpp"

namespace pricers {
    struct Greeks {
        double delta = 0.0; // Sensitivity to underlying price
        double gamma = 0.0; // Sensitivity of delta to underlying price
        double vega = 0.0; // Sensitivity to volatility
        double theta = 0.0; // Sensitivity to time decay (per year)
        double rho = 0.0; // Sensitivity to interest rate
    };

    class AnalyticBS {
    public:
        static double price(const opt::Market& m, const opt::Option& opt);
        static Greeks greeks(const opt::Market& m, const opt::Option& opt);
    
    private:
        static void check_inputs(const opt::Market& m, const opt::Option& opt);
    };
} // namespace pricers