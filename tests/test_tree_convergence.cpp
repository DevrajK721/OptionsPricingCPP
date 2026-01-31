#include "test_framework.hpp"

#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/BinomialCRR.hpp"
#include "pricers/AnalyticBS.hpp"

#include <cmath>
#include <vector>
#include <algorithm>

static double abs_err(double a, double b) { return std::fabs(a - b); }

static void run_convergence_case(const opt::Market& m, const opt::Option& opt,
                                 const std::vector<int>& Ns,
                                 double final_tol, int final_N) {

    const double bs = pricers::AnalyticBS::price(m, opt);

    std::vector<double> errs;
    errs.reserve(Ns.size());

    for (int N : Ns) {
        pricers::TreeParams tp;
        tp.steps = N;
        const double tree = pricers::BinomialCRR::price_european(m, opt, tp);
        errs.push_back(abs_err(tree, bs));
    }

    // Overall improvement
    REQUIRE(errs.back() < 0.7 * errs.front() + 1e-12);

    // Also require best-so-far improves versus early steps
    const double min_err = *std::min_element(errs.begin(), errs.end());
    REQUIRE(min_err < 0.8 * errs.front() + 1e-12);

    // Absolute accuracy at a large N
    pricers::TreeParams tp;
    tp.steps = final_N;
    const double tree_final = pricers::BinomialCRR::price_european(m, opt, tp);
    REQUIRE(abs_err(tree_final, bs) < final_tol);
}

TEST(test_tree_converges_to_bs_call_with_dividends) {
    opt::Market m{100.0, 0.03, 0.01, 0.20};
    opt::Option call{105.0, 0.5, opt::OptionType::Call, opt::Exercise::European};

    std::vector<int> Ns{25, 50, 100, 200, 500, 1000, 2000};

    // CRR typical tolerance
    run_convergence_case(m, call, Ns, /*final_tol=*/5e-3, /*final_N=*/2000);
}

TEST(test_tree_converges_to_bs_put_with_dividends) {
    opt::Market m{100.0, 0.03, 0.01, 0.20};
    opt::Option put{105.0, 1.5, opt::OptionType::Put, opt::Exercise::European};

    std::vector<int> Ns{25, 50, 100, 200, 500, 1000, 2000};

    // Puts + longer maturity can converge slower; allow a looser tol
    run_convergence_case(m, put, Ns, /*final_tol=*/5e-3, /*final_N=*/2000);
}