#include <cmath>

#include "helpers.hpp"
#include "imgedit/ops.hpp"
#include "test.hpp"

using namespace imgedit;

TEST(gaussian_kernel_sums_to_one_and_is_symmetric) {
    auto k = gaussian_kernel_1d(2.0f);
    float sum = 0.0f;
    for (float v : k) sum += v;
    CHECK_NEAR(sum, 1.0, 1e-5);
    CHECK(k.size() == 13);  // radius ceil(3 * 2) = 6
    for (std::size_t i = 0; i < k.size(); ++i) CHECK_NEAR(k[i], k[k.size() - 1 - i], 1e-7);
    CHECK_THROWS(gaussian_kernel_1d(0.0f));
}

TEST(identity_kernel_changes_nothing) {
    Image img = random_image(9, 7, 3);
    Kernel identity{3, {0, 0, 0, 0, 1, 0, 0, 0, 0}};
    CHECK(max_difference(convolve(img, identity), img) < 1e-6f);
}

TEST(even_kernel_is_rejected) {
    Image img(4, 4, 1);
    CHECK_THROWS(convolve(img, Kernel{2, {0.25f, 0.25f, 0.25f, 0.25f}}));
}

TEST(blurring_a_flat_image_changes_nothing) {
    Image img(10, 10, 3, 0.4f);
    CHECK(max_difference(gaussian_blur(img, 2.0f), img) < 1e-5f);
    CHECK(max_difference(box_blur(img, 2), img) < 1e-5f);
    CHECK(max_difference(sharpen(img, 1.5f), img) < 1e-5f);
}

TEST(separable_blur_matches_2d_convolution) {
    Image img = random_image(20, 15, 3);
    CHECK(max_difference(gaussian_blur(img, 1.5f), gaussian_blur_naive(img, 1.5f)) < 1e-5f);
}

TEST(blur_reduces_noise) {
    Image img = random_image(32, 32, 1);
    auto variance = [](const Image& im) {
        double mean = 0, var = 0;
        for (float v : im.data()) mean += v;
        mean /= im.data().size();
        for (float v : im.data()) var += (v - mean) * (v - mean);
        return var / im.data().size();
    };
    CHECK(variance(gaussian_blur(img, 2.0f)) < variance(img) / 4);
}

TEST(sharpen_increases_difference_at_an_edge) {
    Image img(10, 1, 1, 0.3f);
    for (int x = 5; x < 10; ++x) img.at(x, 0, 0) = 0.7f;
    Image s = sharpen(img, 1.0f);
    CHECK(s.at(4, 0, 0) < 0.3f);  // dark side gets darker
    CHECK(s.at(5, 0, 0) > 0.7f);  // bright side gets brighter
}

TEST(sobel_is_zero_on_flat_image) {
    Image img(8, 8, 3, 0.6f);
    Image edges = sobel_edges(img);
    for (float v : edges.data()) CHECK_NEAR(v, 0.0, 1e-6);
}

TEST(sobel_finds_vertical_edge) {
    Image img(8, 8, 1, 0.0f);
    for (int y = 0; y < 8; ++y)
        for (int x = 4; x < 8; ++x) img.at(x, y, 0) = 1.0f;
    Image e = sobel_edges(img);
    CHECK(e.at(3, 4, 0) > 0.5f);         // on the edge
    CHECK_NEAR(e.at(0, 4, 0), 0.0, 1e-6);  // far left, flat
    CHECK_NEAR(e.at(7, 4, 0), 0.0, 1e-6);  // far right, flat
}
