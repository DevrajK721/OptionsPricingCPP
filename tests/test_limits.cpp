#include "test_framework.hpp"
#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"
#include <algorithm>

TEST(test_bs_small_T_intrinsic) {
    opt::Market m{100.0, 0.05, 0.02, 0.30};
    const double T = 1e-6;

    opt::Option call{105.0, T, opt::OptionType::Call, opt::Exercise::European};
    opt::Option put {105.0, T, opt::OptionType::Put,  opt::Exercise::European};

    const double C = pricers::AnalyticBS::price(m, call);
    const double P = pricers::AnalyticBS::price(m, put);

    // intrinsic (very small T, discounting ~ 1)
    const double call_intr = std::max(0.0, m.S0 - call.K);
    const double put_intr  = std::max(0.0, put.K - m.S0);

    REQUIRE_NEAR(C, call_intr, 1e-4);
    REQUIRE_NEAR(P, put_intr,  1e-4);
}