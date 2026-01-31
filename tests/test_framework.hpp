#pragma once
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

inline int g_failures = 0;

#define TEST(name) \
    void name(); \
    int main_##name = (register_test(#name, name), 0); \
    void name()

using TestFn = void(*)();

struct TestEntry { const char* name; TestFn fn; };

inline TestEntry g_tests[256];
inline int g_test_count = 0;

inline void register_test(const char* name, TestFn fn) {
    g_tests[g_test_count++] = TestEntry{name, fn};
}

#define REQUIRE(cond) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ \
                  << " REQUIRE(" #cond ") failed\n"; \
        ++g_failures; \
        return; \
    } \
} while(0)

#define REQUIRE_NEAR(a, b, tol) do { \
    const double _a = (a), _b = (b), _t = (tol); \
    if (std::fabs(_a - _b) > _t) { \
        std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ \
                  << " |" << #a << " - " << #b << "| = " << std::fabs(_a-_b) \
                  << " > " << _t << " (a=" << _a << ", b=" << _b << ")\n"; \
        ++g_failures; \
        return; \
    } \
} while(0)
