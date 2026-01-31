// Market.hpp: Bundle Market Inputs for Black-Scholes 
#pragma once 

namespace opt {
    struct Market {
        double S0 = 0.0; // Current Spot Price 
        double r = 0.0; // Risk-Free Rate 
        double q = 0.0; // Dividend Yield 
        double sigma = 0.0; // Volatility 
    };
} // namespace opt