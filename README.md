# OptionsPricingCPP
Options Pricing Library for European and American Options using CRR Binomial Trees &amp; Black-Scholes with Greeks Computation.

## Unit Tests
Build and run unit tests with 
```bash
g++ -O2 -Iinclude src/pricers/*.cpp tests/test_main.cpp tests/test_parity.cpp tests/test_bounds.cpp tests/test_monotonicity.cpp tests/test_limits.cpp tests/test_tree_convergence.cpp tests/test_american.cpp tests/test_impliedvol.cpp tests/test_greeks.cpp -o build/tests

./build/tests
```

## Usage 
Compile `optcli` client for running Options Pricing Tools using, 
```bash 
g++ -O3 -Iinclude src/pricers/*.cpp src/main.cpp -o build/optcli
```

### Help 
Run help with,
```bash 
./build/optcli --help 
```

### European Option Pricing 
#### Black-Scholes Closed Form Analytic
European Call with Greeks,
```bash 
./build/optcli --style euro --type call --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --sigma 0.25 --greeks
```

European Put with Greeks,
```bash 
./build/optcli --style euro --type put --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --sigma 0.25 --greeks
```

#### Binomial Cox-Ross-Rubinstein (CRR) Tree
The CLI prints both the BS price and the tree price when `--style euro` is used. Choose the tree step count with `--N x` where `x` is an integer.
```bash
./build/optcli --style euro --type call --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --sigma 0.25 --N 2000
```

### American Option Pricing 
#### Binomial CRR Tree 
American Call, 
```bash
./build/optcli --style amer --type call --S0 150 --K 100 --T 1.0 --r 0.01 --q 0.10 --sigma 0.25 --N 2000
```

American Put, 
```bash 
./build/optcli --style amer --type put --S0 100 --K 105 --T 1.0 --r 0.05 --q 0.02 --sigma 0.20 --N 2000
```

### Implied Volatility 
Solve for Black-Scholes Implied Volatility from a target market price `--price`. This is supported for European Options only. 
Implied Volatility for a European Call
```bash 
./build/optcli --style euro --type call --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --iv --price 12.34
```

Implied Volatility for a European Put 
```bash 
./build/optcli --style euro --type put --S0 100 --K 105 --T 1.5 --r 0.03 --q 0.01 --iv --price 14.20
```

## Documentation 
- [Overview](docs/OVERVIEW.md)
- [Math Notes](docs/MATH.md)
- [Numerics & Testing](docs/NUMERICS_AND_TESTING.md)
- [Design](docs/DESIGN.md)