#pragma once

#include <array>
#include <vector>

#include "imgedit/image.hpp"

namespace imgedit {

// ---- color adjustments (adjust.cpp) ----
Image grayscale(const Image& img);
Image brightness(const Image& img, float amount);  // adds amount, e.g. 0.1
Image contrast(const Image& img, float factor);    // 1.0 = no change
Image invert(const Image& img);
Image threshold(const Image& img, float level);    // result is black/white
Image equalize_histogram(const Image& img);        // spreads out the brightness levels
std::array<int, 256> histogram(const Image& img);  // of the luminance

// ---- filters (filters.cpp) ----
// A square kernel of odd size, stored row by row.
struct Kernel {
    int size;
    std::vector<float> weights;
};

Image convolve(const Image& img, const Kernel& kernel);
std::vector<float> gaussian_kernel_1d(float sigma);
Kernel gaussian_kernel_2d(float sigma);

Image box_blur(const Image& img, int radius);
Image gaussian_blur(const Image& img, float sigma);        // separable, O(k) per pixel
Image gaussian_blur_naive(const Image& img, float sigma);  // full 2D kernel, O(k^2) per pixel
Image sharpen(const Image& img, float amount);             // unsharp mask
Image sobel_edges(const Image& img);                       // returns a 1-channel image

// ---- geometry (transform.cpp) ----
Image flip_horizontal(const Image& img);
Image flip_vertical(const Image& img);
Image rotate90(const Image& img);  // clockwise
Image crop(const Image& img, int x, int y, int width, int height);
Image resize_nearest(const Image& img, int new_width, int new_height);
Image resize_bilinear(const Image& img, int new_width, int new_height);

}  // namespace imgedit
