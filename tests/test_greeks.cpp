#include "test_framework.hpp"

#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"

#include <cmath>
#include <algorithm>

static double bump_spot(const opt::Market& m, double newS) {
    opt::Market mm = m;
    mm.S0 = newS;
    return mm.S0;
}

static double price_with(const opt::Market& m, const opt::Option& o) {
    return pricers::AnalyticBS::price(m, o);
}

TEST(test_bs_greeks_fd_call) {
    opt::Market m{100.0, 0.03, 0.01, 0.25};
    opt::Option call{105.0, 1.5, opt::OptionType::Call, opt::Exercise::European};

    const pricers::Greeks g = pricers::AnalyticBS::greeks(m, call);

    // Finite difference step sizes (balanced for stability)
    const double hS = std::max(1e-4 * m.S0, 1e-3);   // spot bump
    const double hV = 1e-4;                         // vol bump
    const double hR = 1e-5;                         // rate bump
    const double hT = 1e-5;                         // time bump (in years)

    // Delta & Gamma via spot bumps (central differences)
    {
        opt::Market m_up = m;  m_up.S0 = m.S0 + hS;
        opt::Market m_dn = m;  m_dn.S0 = m.S0 - hS;

        const double p_up = price_with(m_up, call);
        const double p_0  = price_with(m, call);
        const double p_dn = price_with(m_dn, call);

        const double delta_fd = (p_up - p_dn) / (2.0 * hS);
        const double gamma_fd = (p_up - 2.0 * p_0 + p_dn) / (hS * hS);

        REQUIRE_NEAR(g.delta, delta_fd, 5e-5);
        REQUIRE_NEAR(g.gamma, gamma_fd, 5e-4);
    }

    // Vega via sigma bumps (central difference)
    {
        opt::Market m_up = m; m_up.sigma = m.sigma + hV;
        opt::Market m_dn = m; m_dn.sigma = m.sigma - hV;

        const double p_up = price_with(m_up, call);
        const double p_dn = price_with(m_dn, call);

        const double vega_fd = (p_up - p_dn) / (2.0 * hV);
        REQUIRE_NEAR(g.vega, vega_fd, 5e-4);
    }

    // Rho via r bumps (central difference)
    {
        opt::Market m_up = m; m_up.r = m.r + hR;
        opt::Market m_dn = m; m_dn.r = m.r - hR;

        const double p_up = price_with(m_up, call);
        const double p_dn = price_with(m_dn, call);

        const double rho_fd = (p_up - p_dn) / (2.0 * hR);
        REQUIRE_NEAR(g.rho, rho_fd, 5e-4);
    }

    // Theta: by convention coded -dV/dt calendar. 
    {
        opt::Option o_up = call; o_up.T = call.T + hT;
        opt::Option o_dn = call; o_dn.T = call.T - hT;

        const double p_up = price_with(m, o_up);
        const double p_dn = price_with(m, o_dn);

        const double theta_fd = -(p_up - p_dn) / (2.0 * hT);
        REQUIRE_NEAR(g.theta, theta_fd, 5e-4);
    }

    // Sanity checks
    REQUIRE(g.gamma > 0.0);
    REQUIRE(g.vega  > 0.0);
    REQUIRE(g.delta > 0.0);
    REQUIRE(g.delta < 1.0);
}

TEST(test_bs_greeks_fd_put) {
    opt::Market m{100.0, 0.03, 0.01, 0.25};
    opt::Option put{105.0, 1.5, opt::OptionType::Put, opt::Exercise::European};

    const pricers::Greeks g = pricers::AnalyticBS::greeks(m, put);

    const double hS = std::max(1e-4 * m.S0, 1e-3);
    const double hV = 1e-4;

    // Put delta should be in (-1, 0)
    REQUIRE(g.delta < 0.0);
    REQUIRE(g.delta > -1.0);

    // Gamma & Vega positive
    REQUIRE(g.gamma > 0.0);
    REQUIRE(g.vega  > 0.0);

    // FD check for delta/vega (enough to catch sign mistakes)
    {
        opt::Market m_up = m;  m_up.S0 = m.S0 + hS;
        opt::Market m_dn = m;  m_dn.S0 = m.S0 - hS;

        const double p_up = price_with(m_up, put);
        const double p_dn = price_with(m_dn, put);

        const double delta_fd = (p_up - p_dn) / (2.0 * hS);
        REQUIRE_NEAR(g.delta, delta_fd, 5e-5);
    }
    {
        opt::Market m_up = m; m_up.sigma = m.sigma + hV;
        opt::Market m_dn = m; m_dn.sigma = m.sigma - hV;

        const double p_up = price_with(m_up, put);
        const double p_dn = price_with(m_dn, put);

        const double vega_fd = (p_up - p_dn) / (2.0 * hV);
        REQUIRE_NEAR(g.vega, vega_fd, 5e-4);
    }
}

TEST(test_bs_greek_parity_relations) {
    // Put-call parity derivatives:
    // C - P = S e^{-qT} - K e^{-rT}
    // d/dS: Delta_C - Delta_P = e^{-qT}
    // d^2/dS^2: Gamma_C - Gamma_P = 0
    // d/dsigma: Vega_C - Vega_P = 0

    opt::Market m{120.0, 0.04, 0.02, 0.30};
    opt::Option call{110.0, 0.8, opt::OptionType::Call, opt::Exercise::European};
    opt::Option put {110.0, 0.8, opt::OptionType::Put,  opt::Exercise::European};

    const auto gc = pricers::AnalyticBS::greeks(m, call);
    const auto gp = pricers::AnalyticBS::greeks(m, put);

    const double eqT = std::exp(-m.q * call.T);

    REQUIRE_NEAR(gc.delta - gp.delta, eqT, 1e-10);
    REQUIRE_NEAR(gc.gamma - gp.gamma, 0.0, 1e-10);
    REQUIRE_NEAR(gc.vega  - gp.vega,  0.0, 1e-10);
}