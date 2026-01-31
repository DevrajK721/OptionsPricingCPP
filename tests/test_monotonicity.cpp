#include "test_framework.hpp"
#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"

TEST(test_bs_monotonicity) {
    const double K = 100.0;
    const double T = 1.0;

    opt::Option call{K, T, opt::OptionType::Call, opt::Exercise::European};
    opt::Option put {K, T, opt::OptionType::Put,  opt::Exercise::European};

    opt::Market m0{100.0, 0.02, 0.00, 0.20};

    // Spot up => call up
    opt::Market mS_up = m0;
    mS_up.S0 = 110.0;
    REQUIRE(pricers::AnalyticBS::price(mS_up, call) > pricers::AnalyticBS::price(m0, call));

    // Strike up => call down
    opt::Option callK_up{110.0, T, opt::OptionType::Call, opt::Exercise::European};
    REQUIRE(pricers::AnalyticBS::price(m0, callK_up) < pricers::AnalyticBS::price(m0, call));

    // Vol up => call and put up
    opt::Market mVol_up = m0;
    mVol_up.sigma = 0.40;
    REQUIRE(pricers::AnalyticBS::price(mVol_up, call) > pricers::AnalyticBS::price(m0, call));
    REQUIRE(pricers::AnalyticBS::price(mVol_up, put)  > pricers::AnalyticBS::price(m0, put));
}