#include "test_framework.hpp"
#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"
#include <cmath>

TEST(test_put_call_parity_bs) {
    const double S = 100.0;
    const double K = 105.0;
    const double r = 0.03;
    const double q = 0.01;
    const double sigma = 0.20;
    const double T = 1.5;

    opt::Market m{S, r, q, sigma};
    opt::Option call{K, T, opt::OptionType::Call, opt::Exercise::European};
    opt::Option put {K, T, opt::OptionType::Put,  opt::Exercise::European};

    const double C = pricers::AnalyticBS::price(m, call);
    const double P = pricers::AnalyticBS::price(m, put);

    const double rhs = S * std::exp(-q * T) - K * std::exp(-r * T);
    REQUIRE_NEAR(C - P, rhs, 1e-10);
}