#include <cstdio>

#include "test.hpp"

int main() {
    int failed_tests = 0;
    for (const auto& c : test::registry()) {
        int before = test::failures();
        c.fn();
        bool ok = test::failures() == before;
        if (!ok) ++failed_tests;
        std::printf("[%s] %s\n", ok ? " OK " : "FAIL", c.name.c_str());
    }
    std::printf("\n%zu tests, %d failed\n", test::registry().size(), failed_tests);
    return failed_tests == 0 ? 0 : 1;
}
