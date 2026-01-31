#include "test_framework.hpp"

#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"
#include "pricers/ImpliedVol.hpp"

#include <cmath>
#include <algorithm>

static double call_lower_bound(const opt::Market& m, const opt::Option& opt) {
    const double Dr = std::exp(-m.r * opt.T);
    const double Dq = std::exp(-m.q * opt.T);
    return std::max(0.0, m.S0 * Dq - opt.K * Dr);
}

static double call_upper_bound(const opt::Market& m, const opt::Option& opt) {
    const double Dq = std::exp(-m.q * opt.T);
    return m.S0 * Dq;
}

static double put_lower_bound(const opt::Market& m, const opt::Option& opt) {
    const double Dr = std::exp(-m.r * opt.T);
    const double Dq = std::exp(-m.q * opt.T);
    return std::max(0.0, opt.K * Dr - m.S0 * Dq);
}

static double put_upper_bound(const opt::Market& m, const opt::Option& opt) {
    const double Dr = std::exp(-m.r * opt.T);
    return opt.K * Dr;
}

static bool did_throw_implied_vol(const opt::Market& m, const opt::Option& opt, double target) {
    try {
        (void)pricers::ImpliedVol::solve_bs(m, opt, target);
        return false;
    } catch (...) {
        return true;
    }
}

TEST(test_implied_vol_recovers_sigma_call_with_dividends) {
    // Choose a nontrivial case: q != 0, not ATM, moderate T.
    const double sigma_true = 0.27;
    opt::Market m{100.0, 0.03, 0.01, sigma_true};
    opt::Option call{105.0, 1.5, opt::OptionType::Call, opt::Exercise::European};

    const double target_price = pricers::AnalyticBS::price(m, call);

    // The solver overwrites sigma internally, so sigma here is irrelevant
    opt::Market m_guess = m;
    m_guess.sigma = 0.10;

    const double sigma_hat = pricers::ImpliedVol::solve_bs(m_guess, call, target_price);
    REQUIRE_NEAR(sigma_hat, sigma_true, 1e-6);
}

TEST(test_implied_vol_recovers_sigma_put_with_dividends) {
    const double sigma_true = 0.35;
    opt::Market m{100.0, 0.05, 0.02, sigma_true};
    opt::Option put{90.0, 0.75, opt::OptionType::Put, opt::Exercise::European};

    const double target_price = pricers::AnalyticBS::price(m, put);

    opt::Market m_guess = m;
    m_guess.sigma = 0.12;

    const double sigma_hat = pricers::ImpliedVol::solve_bs(m_guess, put, target_price);
    REQUIRE_NEAR(sigma_hat, sigma_true, 1e-6);
}

TEST(test_implied_vol_throws_below_lower_bound_call) {
    opt::Market m{100.0, 0.03, 0.01, 0.20};
    opt::Option call{105.0, 1.0, opt::OptionType::Call, opt::Exercise::European};

    const double lb = call_lower_bound(m, call);
    const double bad_price = lb - 1e-3;

    REQUIRE(did_throw_implied_vol(m, call, bad_price));
}

TEST(test_implied_vol_throws_above_upper_bound_call) {
    opt::Market m{100.0, 0.03, 0.01, 0.20};
    opt::Option call{105.0, 1.0, opt::OptionType::Call, opt::Exercise::European};

    const double ub = call_upper_bound(m, call);
    const double bad_price = ub + 1e-3;

    REQUIRE(did_throw_implied_vol(m, call, bad_price));
}

TEST(test_implied_vol_throws_below_lower_bound_put) {
    opt::Market m{100.0, 0.03, 0.01, 0.20};
    opt::Option put{105.0, 1.0, opt::OptionType::Put, opt::Exercise::European};

    const double lb = put_lower_bound(m, put);
    const double bad_price = lb - 1e-3;

    REQUIRE(did_throw_implied_vol(m, put, bad_price));
}

TEST(test_implied_vol_throws_above_upper_bound_put) {
    opt::Market m{100.0, 0.03, 0.01, 0.20};
    opt::Option put{105.0, 1.0, opt::OptionType::Put, opt::Exercise::European};

    const double ub = put_upper_bound(m, put);
    const double bad_price = ub + 1e-3;

    REQUIRE(did_throw_implied_vol(m, put, bad_price));
}

TEST(test_implied_vol_near_lower_bound_returns_small_sigma) {
    // Deep OTM call, very low price -> implied vol should be very small
    opt::Market m{100.0, 0.03, 0.00, 0.20};
    opt::Option call{200.0, 0.5, opt::OptionType::Call, opt::Exercise::European};

    const double lb = call_lower_bound(m, call); // likely 0
    const double target_price = lb;              // exactly at lower bound

    pricers::ImpliedVolParams params;
    params.sigma_lo = 1e-8;

    const double sigma_hat = pricers::ImpliedVol::solve_bs(m, call, target_price, params);
    REQUIRE(sigma_hat <= 1e-6); // should come back extremely small
}