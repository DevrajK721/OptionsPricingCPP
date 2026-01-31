#include "opt/Market.hpp"
#include "opt/Option.hpp"
#include "pricers/AnalyticBS.hpp"
#include "pricers/BinomialCRR.hpp"
#include "pricers/ImpliedVol.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib>

static void print_usage() {
    std::cout <<
    R"(Usage:
    optcli --style [euro|amer] --type [call|put] --S0 <spot> --K <strike> --T <years>
            --r <rate> --q <div_yield> [--sigma <vol>] [--N <steps>]
            [--greeks] [--iv --price <target_price>]

    Examples:
    optcli --style euro --type call --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --sigma 0.25 --N 2000 --greeks
    optcli --style amer --type put  --S0 100 --K 105 --T 1.0 --r 0.05 --q 0.02 --sigma 0.20 --N 2000
    optcli --style euro --type call --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --iv --price 12.34

    Notes:
    - European: prints BS analytic + CRR tree price.
    - American: prints CRR tree price only.
    - --greeks uses BS analytic Greeks (European only).
    - --iv solves BS implied volatility from --price (European only).
    )";
}

static std::unordered_map<std::string, std::string> parse_kv(int argc, char** argv) {
    std::unordered_map<std::string, std::string> kv;
    for (int i = 1; i < argc; ++i) {
        std::string key = argv[i];
        if (key.rfind("--", 0) != 0) {
            throw std::invalid_argument("Expected flag starting with --, got: " + key);
        }
        // boolean flags
        if (key == "--greeks" || key == "--iv" || key == "--help") {
            kv[key] = "1";
            continue;
        }
        if (i + 1 >= argc) throw std::invalid_argument("Missing value after: " + key);
        kv[key] = argv[++i];
    }
    return kv;
}

static bool has(const std::unordered_map<std::string, std::string>& kv, const std::string& k) {
    return kv.find(k) != kv.end();
}

static std::string get_str(const std::unordered_map<std::string, std::string>& kv,
                           const std::string& k,
                           const std::string& def = "") {
    auto it = kv.find(k);
    if (it == kv.end()) {
        if (def != "") return def;
        throw std::invalid_argument("Missing required flag: " + k);
    }
    return it->second;
}

static double get_d(const std::unordered_map<std::string, std::string>& kv,
                    const std::string& k,
                    bool required = true,
                    double def = 0.0) {
    auto it = kv.find(k);
    if (it == kv.end()) {
        if (required) throw std::invalid_argument("Missing required flag: " + k);
        return def;
    }
    return std::stod(it->second);
}

static int get_i(const std::unordered_map<std::string, std::string>& kv,
                 const std::string& k,
                 bool required = true,
                 int def = 0) {
    auto it = kv.find(k);
    if (it == kv.end()) {
        if (required) throw std::invalid_argument("Missing required flag: " + k);
        return def;
    }
    return std::stoi(it->second);
}

static opt::OptionType parse_type(const std::string& s) {
    if (s == "call") return opt::OptionType::Call;
    if (s == "put")  return opt::OptionType::Put;
    throw std::invalid_argument("Invalid --type (use call|put): " + s);
}

static opt::Exercise parse_style(const std::string& s) {
    if (s == "euro") return opt::Exercise::European;
    if (s == "amer") return opt::Exercise::American;
    throw std::invalid_argument("Invalid --style (use euro|amer): " + s);
}

int main(int argc, char** argv) {
    try {
        auto kv = parse_kv(argc, argv);

        if (argc == 1 || has(kv, "--help")) {
            print_usage();
            return 0;
        }

        const auto style = parse_style(get_str(kv, "--style"));
        const auto type  = parse_type(get_str(kv, "--type"));

        const double S0 = get_d(kv, "--S0");
        const double K  = get_d(kv, "--K");
        const double T  = get_d(kv, "--T");
        const double r  = get_d(kv, "--r");
        const double q  = get_d(kv, "--q");

        const int N = get_i(kv, "--N", /*required=*/false, /*def=*/2000);

        const bool want_greeks = has(kv, "--greeks");
        const bool want_iv     = has(kv, "--iv");

        // sigma is required unless we are doing implied vol
        const double sigma = want_iv ? get_d(kv, "--sigma", /*required=*/false, /*def=*/0.20)
                                     : get_d(kv, "--sigma", /*required=*/true);

        opt::Market m{S0, r, q, sigma};
        opt::Option o{K, T, type, style};

        std::cout << std::fixed << std::setprecision(6);

        // ---- Implied vol path (European only) ----
        if (want_iv) {
            if (style != opt::Exercise::European) {
                throw std::invalid_argument("--iv is only supported for European options (BS).");
            }
            if (!has(kv, "--price")) {
                throw std::invalid_argument("--iv requires --price <target_price>.");
            }
            const double target = get_d(kv, "--price");

            pricers::ImpliedVolParams p; // defaults ok
            const double iv = pricers::ImpliedVol::solve_bs(m, o, target, p);

            std::cout << "Implied vol (BS): " << iv << "\n";
            return 0;
        }

        // ---- Pricing ----
        pricers::TreeParams tp;
        tp.steps = N;

        if (style == opt::Exercise::European) {
            const double bs   = pricers::AnalyticBS::price(m, o);
            const double tree = pricers::BinomialCRR::price_european(m, o, tp);

            std::cout << "European " << (type == opt::OptionType::Call ? "Call" : "Put") << "\n";
            std::cout << "BS price:   " << bs   << "\n";
            std::cout << "Tree price: " << tree << " (N=" << N << ")\n";

            if (want_greeks) {
                const auto g = pricers::AnalyticBS::greeks(m, o);
                std::cout << "\nGreeks (BS, per unit):\n";
                std::cout << "Delta: " << g.delta << "\n";
                std::cout << "Gamma: " << g.gamma << "\n";
                std::cout << "Vega:  " << g.vega  << "\n";
                std::cout << "Theta: " << g.theta << " (calendar theta, per year)\n";
                std::cout << "Rho:   " << g.rho   << "\n";
            }
        } else {
            const double amer = pricers::BinomialCRR::price_american(m, o, tp);

            std::cout << "American " << (type == opt::OptionType::Call ? "Call" : "Put") << "\n";
            std::cout << "Tree price: " << amer << " (N=" << N << ")\n";

            if (want_greeks) {
                std::cout << "\nNote: Greeks via BS are for European options only.\n";
            }
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[error] " << e.what() << "\n\n";
        print_usage();
        return 1;
    }
}