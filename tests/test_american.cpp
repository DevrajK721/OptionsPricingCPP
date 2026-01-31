#include "test_framework.hpp"

#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/BinomialCRR.hpp"
#include "pricers/AnalyticBS.hpp"

#include <cmath>
#include <vector>
#include <algorithm>

static double abs_err(double a, double b) { return std::fabs(a - b); }

TEST(test_american_put_higher_than_european) {
    opt::Market m{150.0, 0.05, 0.02, 0.20};
    opt::Option put_amer{100.0, 1.0, opt::OptionType::Put, opt::Exercise::American};
    opt::Option put_euro{100.0, 1.0, opt::OptionType::Put, opt::Exercise::European};

    pricers::TreeParams tp;
    tp.steps = 2000;

    double price_amer = pricers::BinomialCRR::price_american(m, put_amer, tp);
    double price_euro_analytic = pricers::AnalyticBS::price(m, put_euro);
    double price_euro_tree = pricers::BinomialCRR::price_european(m, put_euro, tp);

    REQUIRE(price_amer >= price_euro_analytic - 5e-3);
    REQUIRE(price_amer >= price_euro_tree - 1e-12);
}

TEST(test_american_call_equals_european_no_dividends) {
    opt::Market m{150.0, 0.05, 0.0, 0.20};
    opt::Option call_amer{100.0, 1.0, opt::OptionType::Call, opt::Exercise::American};
    opt::Option call_euro{100.0, 1.0, opt::OptionType::Call, opt::Exercise::European};

    pricers::TreeParams tp;
    tp.steps = 2000;
    double price_amer = pricers::BinomialCRR::price_american(m, call_amer, tp);
    double price_euro_analytic = pricers::AnalyticBS::price(m, call_euro);
    double price_euro_tree = pricers::BinomialCRR::price_european(m, call_euro, tp);

    REQUIRE(std::fabs(price_amer - price_euro_analytic) < 5e-3);
    REQUIRE(std::fabs(price_amer - price_euro_tree) < 1e-7);
}   

TEST(test_american_call_greater_than_european_with_dividends) {
    opt::Market m{150.0, 0.01, 0.10, 1.0};
    opt::Option call_amer{100.0, 1.0, opt::OptionType::Call, opt::Exercise::American};
    opt::Option call_euro{100.0, 1.0, opt::OptionType::Call, opt::Exercise::European};

    pricers::TreeParams tp;
    tp.steps = 2000;

    double price_amer = pricers::BinomialCRR::price_american(m, call_amer, tp);
    double price_euro_analytic = pricers::AnalyticBS::price(m, call_euro);
    double price_euro_tree = pricers::BinomialCRR::price_european(m, call_euro, tp);

    REQUIRE(price_amer >= price_euro_analytic - 1e-4);
    REQUIRE(price_amer >= price_euro_tree + 1e-4);
}