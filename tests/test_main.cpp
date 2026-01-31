#include "test_framework.hpp"
#include <iostream>

int main() {
    std::cout << "Running " << g_test_count << " tests...\n";

    for (int i = 0; i < g_test_count; ++i) {
        std::cout << " - " << g_tests[i].name << "\n";
        g_tests[i].fn();
    }

    if (g_failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << g_failures << " test(s) failed.\n";
    return 1;
}