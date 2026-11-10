// A tiny test framework so the project has no external dependencies.
// TEST(name) registers a test, CHECK(...) records a failure without stopping.
#pragma once

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace test {

struct Case {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<Case>& registry() {
    static std::vector<Case> cases;
    return cases;
}

inline int& failures() {
    static int count = 0;
    return count;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) { registry().push_back({name, fn}); }
};

inline void fail(const char* file, int line, const std::string& msg) {
    std::printf("    %s:%d: %s\n", file, line, msg.c_str());
    ++failures();
}

}  // namespace test

#define TEST_CONCAT2(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT2(a, b)

#define TEST(name)                                                              \
    static void name();                                                         \
    static test::Registrar TEST_CONCAT(registrar_, name)(#name, name);          \
    static void name()

#define CHECK(cond)                                                             \
    do {                                                                        \
        if (!(cond)) test::fail(__FILE__, __LINE__, "CHECK(" #cond ") failed"); \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                   \
    do {                                                                        \
        double va = (a), vb = (b);                                              \
        if (std::fabs(va - vb) > (eps))                                         \
            test::fail(__FILE__, __LINE__,                                      \
                       #a " = " + std::to_string(va) + ", expected " +          \
                           std::to_string(vb));                                 \
    } while (0)

#define CHECK_THROWS(expr)                                                      \
    do {                                                                        \
        bool thrown = false;                                                    \
        try {                                                                   \
            (void)(expr);                                                       \
        } catch (...) {                                                         \
            thrown = true;                                                      \
        }                                                                       \
        if (!thrown) test::fail(__FILE__, __LINE__, #expr " did not throw");    \
    } while (0)
