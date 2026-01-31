// Math.hpp: Mathematical utility functions
#pragma once
#include <cmath> 
#include <algorithm>

namespace util {
    // Normal PDF 
    inline double normal_pdf(double x) {
        static constexpr double INV_SQRT_2PI = 0.398942280401432677939946059934; // 1/sqrt(2pi)
        return INV_SQRT_2PI * std::exp(-0.5 * x * x); 
    }

    // Normal CDF
    inline double normal_cdf(double x) {
        return 0.5 * std::erfc(-x / std::sqrt(2.0));
    }
    
    // Clamp Function 
    inline double clamp(double x, double lo, double hi) {
        return std::max(lo, std::min(x, hi));
    }
} // namespace util