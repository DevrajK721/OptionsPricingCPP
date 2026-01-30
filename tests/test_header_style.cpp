// Compile-only test: each header must be self-contained.
#include "opt/Types.hpp"
#include "opt/Payoff.hpp"
#include "opt/Option.hpp"
#include "opt/Market.hpp"
#include "pricers/AnalyticBS.hpp"
#include "pricers/BinomialCRR.hpp"
#include "pricers/ImpliedVol.hpp"
#include "pde/CrankNicolson.hpp"
#include "pde/Tridiagonal.hpp"
#include "util/Math.hpp"
#include "util/Args.hpp"
#include "util/Timer.hpp"

int main() { return 0; }