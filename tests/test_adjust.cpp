#include "helpers.hpp"
#include "imgedit/ops.hpp"
#include "test.hpp"

using namespace imgedit;

TEST(grayscale_uses_luminance_weights) {
    Image img(1, 1, 3);
    img.at(0, 0, 0) = 1.0f;  // pure red
    Image g = grayscale(img);
    CHECK(g.channels() == 1);
    CHECK_NEAR(g.at(0, 0, 0), 0.299, 1e-6);
}

TEST(invert_twice_gives_original) {
    Image img = random_image(8, 8, 3);
    CHECK(max_difference(invert(invert(img)), img) < 1e-6f);
}

TEST(invert_keeps_alpha) {
    Image img(1, 1, 4, 0.2f);
    Image out = invert(img);
    CHECK_NEAR(out.at(0, 0, 0), 0.8, 1e-6);
    CHECK_NEAR(out.at(0, 0, 3), 0.2, 1e-6);
}

TEST(brightness_is_clamped) {
    Image img(1, 1, 1, 0.9f);
    CHECK_NEAR(brightness(img, 0.5f).at(0, 0, 0), 1.0, 1e-6);
    CHECK_NEAR(brightness(img, -2.0f).at(0, 0, 0), 0.0, 1e-6);
}

TEST(contrast_of_one_changes_nothing) {
    Image img = random_image(6, 6, 3);
    CHECK(max_difference(contrast(img, 1.0f), img) < 1e-6f);
}

TEST(contrast_pushes_values_away_from_gray) {
    Image img(1, 1, 1, 0.6f);
    CHECK_NEAR(contrast(img, 2.0f).at(0, 0, 0), 0.7, 1e-6);
}

TEST(threshold_gives_black_and_white) {
    Image img(2, 1, 1);
    img.at(0, 0, 0) = 0.3f;
    img.at(1, 0, 0) = 0.7f;
    Image t = threshold(img, 0.5f);
    CHECK_NEAR(t.at(0, 0, 0), 0.0, 1e-6);
    CHECK_NEAR(t.at(1, 0, 0), 1.0, 1e-6);
}

TEST(histogram_counts_every_pixel) {
    Image img = random_image(10, 10, 3);
    auto h = histogram(img);
    int total = 0;
    for (int v : h) total += v;
    CHECK(total == 100);
}

TEST(equalize_stretches_two_levels_to_full_range) {
    // A dull image with only two close gray levels.
    Image img(4, 1, 1);
    img.at(0, 0, 0) = img.at(1, 0, 0) = 0.4f;
    img.at(2, 0, 0) = img.at(3, 0, 0) = 0.5f;
    Image eq = equalize_histogram(img);
    CHECK_NEAR(eq.at(0, 0, 0), 0.0, 1e-6);
    CHECK_NEAR(eq.at(3, 0, 0), 1.0, 1e-6);
}

TEST(equalize_leaves_flat_image_alone) {
    Image img(3, 3, 3, 0.5f);
    CHECK(max_difference(equalize_histogram(img), img) < 1e-6f);
}
