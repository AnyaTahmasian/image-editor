// Compares the separable gaussian blur with the naive 2D convolution.
// Build in Release mode for meaningful numbers:
//   cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ./build/imgedit_bench

#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>

#include "imgedit/ops.hpp"

using namespace imgedit;

template <typename F>
double time_ms(F f) {
    auto start = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    const int size = 512;
    Image img(size, size, 3);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (float& v : img.data()) v = dist(rng);

    std::printf("%dx%d RGB image\n\n", size, size);
    std::printf("%6s %8s %14s %14s %9s\n", "sigma", "kernel", "naive (ms)", "separable (ms)",
                "speedup");

    for (float sigma : {1.0f, 2.0f, 4.0f, 8.0f}) {
        Image a, b;
        double naive = time_ms([&] { a = gaussian_blur_naive(img, sigma); });
        double sep = time_ms([&] { b = gaussian_blur(img, sigma); });

        // Both must give the same picture, otherwise the comparison means nothing.
        float max_diff = 0.0f;
        for (std::size_t i = 0; i < a.data().size(); ++i)
            max_diff = std::fmax(max_diff, std::fabs(a.data()[i] - b.data()[i]));
        if (max_diff > 1e-4f) {
            std::printf("results differ by %g\n", max_diff);
            return 1;
        }

        int k = static_cast<int>(gaussian_kernel_1d(sigma).size());
        std::printf("%6.1f %5dx%-3d %14.1f %14.1f %8.1fx\n", sigma, k, k, naive, sep, naive / sep);
    }
    return 0;
}
