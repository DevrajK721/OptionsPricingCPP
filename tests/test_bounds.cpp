// test_bounds.cpp: Unit tests for option bounds calculations
#include "test_framework.hpp"
#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"
#include "pricers/BinomialCRR.hpp"
#include <cmath> 

TEST(test_bs_bounds) {
    opt::Market m{100.0, 0.03, 0.01, 0.2};
    const double K = 105.0;
    const double T = 1.5;

    opt::Option call{K, T, opt::OptionType::Call, opt::Exercise::European};
    opt::Option put{K, T, opt::OptionType::Put, opt::Exercise::European};

    const double C = pricers::AnalyticBS::price(m, call);
    const double P = pricers::AnalyticBS::price(m, put);

    const double discFactorR = std::exp(-m.r * T);
    const double discFactorQ = std::exp(-m.q * T);

    const double callLB = std::max(0.0, m.S0 * discFactorQ - K * discFactorR);
    const double callUB = K * discFactorR;

    const double putLB = std::max(0.0, K * discFactorR - m.S0 * discFactorQ);
    const double putUB = K * discFactorR;

    REQUIRE(C >= callLB - 1e-12);
    REQUIRE(C <= callUB + 1e-12);

    REQUIRE(P >= putLB - 1e-12);
    REQUIRE(P <= putUB + 1e-12);
}

TEST(test_american_bounds) {
    opt::Market m{150.0, 0.05, 0.02, 0.2};
    opt::Option call{100.0, 1.0, opt::OptionType::Call, opt::Exercise::American};
    opt::Option put{100.0, 1.0, opt::OptionType::Put, opt::Exercise::American};

    pricers::TreeParams tp;
    tp.steps = 2000;

    const double C = pricers::BinomialCRR::price_american(m, call, tp);
    const double P = pricers::BinomialCRR::price_american(m, put, tp);

    REQUIRE(C >= std::max(0.0, m.S0 - 100.0) - 1e-12); // Call lower bound
    REQUIRE(P >= std::max(0.0, 100.0 - m.S0) - 1e-12); // Put lower bound

    REQUIRE(C <= m.S0 + 1e-12); // Call upper bound
    REQUIRE(P <= 100.0 + 1e-12); // Put upper bound

}