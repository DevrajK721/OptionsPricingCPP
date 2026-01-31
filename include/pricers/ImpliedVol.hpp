#pragma once
#include "opt/Market.hpp"
#include "opt/Option.hpp"

namespace pricers {

struct ImpliedVolParams {
    double sigma_lo = 1e-8;
    double sigma_hi = 2.0;      // initial guess; you can auto-expand
    double tol_sigma = 1e-8;
    double tol_price = 1e-10;
    int max_iter = 200;
};

class ImpliedVol {
public:
    static double solve_bs(const opt::Market& m,
                           const opt::Option& opt,
                           double target_price,
                           const ImpliedVolParams& params = ImpliedVolParams{});

private:
    static void check_inputs(const opt::Market& m,
                             const opt::Option& opt,
                             double target_price);

    static void bs_bounds(const opt::Market& m,
                          const opt::Option& opt,
                          double& lower,
                          double& upper);
};

} // namespace pricers