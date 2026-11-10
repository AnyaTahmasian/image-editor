#include <cmath>

#include "imgedit/ops.hpp"

namespace imgedit {

namespace {

// Runs a 1D kernel along one axis (dx, dy) = (1, 0) for rows, (0, 1) for columns.
Image convolve_1d(const Image& img, const std::vector<float>& k, int dx, int dy) {
    const int r = static_cast<int>(k.size()) / 2;
    Image out(img.width(), img.height(), img.channels());
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            for (int c = 0; c < img.channels(); ++c) {
                float sum = 0.0f;
                for (int i = -r; i <= r; ++i) {
                    sum += k[i + r] * img.clamped(x + i * dx, y + i * dy, c);
                }
                out.at(x, y, c) = sum;
            }
    return out;
}

}  // namespace

Image convolve(const Image& img, const Kernel& kernel) {
    if (kernel.size % 2 == 0 || kernel.weights.size() != static_cast<size_t>(kernel.size * kernel.size)) {
        throw ImageError("kernel must be square with an odd size");
    }
    const int r = kernel.size / 2;
    Image out(img.width(), img.height(), img.channels());
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            for (int c = 0; c < img.channels(); ++c) {
                float sum = 0.0f;
                for (int ky = -r; ky <= r; ++ky)
                    for (int kx = -r; kx <= r; ++kx) {
                        float w = kernel.weights[(ky + r) * kernel.size + (kx + r)];
                        sum += w * img.clamped(x + kx, y + ky, c);
                    }
                out.at(x, y, c) = sum;
            }
    return out;
}

std::vector<float> gaussian_kernel_1d(float sigma) {
    if (sigma <= 0.0f) throw ImageError("sigma must be positive");
    // 3 sigma on each side covers ~99.7% of the bell curve.
    const int r = static_cast<int>(std::ceil(3.0f * sigma));
    std::vector<float> k(2 * r + 1);
    float sum = 0.0f;
    for (int i = -r; i <= r; ++i) {
        k[i + r] = std::exp(-(i * i) / (2.0f * sigma * sigma));
        sum += k[i + r];
    }
    for (float& v : k) v /= sum;  // weights add up to 1 so brightness is kept
    return k;
}

Kernel gaussian_kernel_2d(float sigma) {
    // A 2D gaussian is the outer product of two 1D gaussians.
    auto k1 = gaussian_kernel_1d(sigma);
    const int n = static_cast<int>(k1.size());
    Kernel k{n, std::vector<float>(n * n)};
    for (int y = 0; y < n; ++y)
        for (int x = 0; x < n; ++x)
            k.weights[y * n + x] = k1[y] * k1[x];
    return k;
}

Image gaussian_blur(const Image& img, float sigma) {
    // Separable: blur rows, then columns. Same result as the 2D kernel
    // but 2k multiplications per pixel instead of k*k.
    auto k = gaussian_kernel_1d(sigma);
    return convolve_1d(convolve_1d(img, k, 1, 0), k, 0, 1);
}

Image gaussian_blur_naive(const Image& img, float sigma) {
    return convolve(img, gaussian_kernel_2d(sigma));
}

Image box_blur(const Image& img, int radius) {
    if (radius < 1) throw ImageError("blur radius must be at least 1");
    std::vector<float> k(2 * radius + 1, 1.0f / (2 * radius + 1));
    return convolve_1d(convolve_1d(img, k, 1, 0), k, 0, 1);
}

Image sharpen(const Image& img, float amount) {
    // Unsharp mask: original + amount * (original - blurred).
    // (original - blurred) is the fine detail, so this boosts the detail.
    Image blurred = gaussian_blur(img, 1.0f);
    Image out = img;
    for (std::size_t i = 0; i < out.data().size(); ++i) {
        float detail = img.data()[i] - blurred.data()[i];
        out.data()[i] = std::fmin(1.0f, std::fmax(0.0f, img.data()[i] + amount * detail));
    }
    return out;
}

Image sobel_edges(const Image& img) {
    // Two 3x3 kernels estimate the brightness gradient in x and y.
    // The edge strength is the length of the gradient vector.
    static const Kernel gx{3, {-1, 0, 1, -2, 0, 2, -1, 0, 1}};
    static const Kernel gy{3, {-1, -2, -1, 0, 0, 0, 1, 2, 1}};

    Image gray = grayscale(img);
    Image dx = convolve(gray, gx);
    Image dy = convolve(gray, gy);

    Image out(img.width(), img.height(), 1);
    float strongest = 0.0f;
    for (std::size_t i = 0; i < out.data().size(); ++i) {
        out.data()[i] = std::hypot(dx.data()[i], dy.data()[i]);
        strongest = std::fmax(strongest, out.data()[i]);
    }
    // Scale so the strongest edge is white, otherwise edges in normal photos look very faint.
    if (strongest > 0.0f) {
        for (float& v : out.data()) v /= strongest;
    }
    return out;
}

}  // namespace imgedit
