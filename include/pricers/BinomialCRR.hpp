// BinonialCRR.hpp: Binomial Cox-Ross-Rubinstein (CRR) option pricing model
#pragma once
#include "opt/Market.hpp"
#include "opt/Option.hpp"

namespace pricers {

struct TreeParams {
    int steps = 200;   // N
};

class BinomialCRR {
public:
    // European via backward induction (no early exercise)
    static double price_european(const opt::Market& m,
                                 const opt::Option& opt,
                                 const TreeParams& p);

    // American via backward induction + early exercise max()
    static double price_american(const opt::Market& m,
                                 const opt::Option& opt,
                                 const TreeParams& p);

private:
    static void check_inputs(const opt::Market& m,
                             const opt::Option& opt,
                             const TreeParams& p);

    // Helper to compute u,d,p,dt and discount factors
    struct CRRCoefs {
        double dt = 0.0;
        double u  = 0.0;
        double d  = 0.0;
        double pu = 0.0;  // risk-neutral prob of up
        double pd = 0.0;  // 1 - pu
        double disc = 0.0; // exp(-r*dt)
    };

    static CRRCoefs make_coefs(const opt::Market& m,
                               const opt::Option& opt,
                               const TreeParams& p);

    // payoff at stock price S (vanilla only for now)
    static double payoff(double S, const opt::Option& opt);
};

} // namespace pricers