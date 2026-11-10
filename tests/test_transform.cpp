#include "helpers.hpp"
#include "imgedit/ops.hpp"
#include "test.hpp"

using namespace imgedit;

TEST(flipping_twice_gives_original) {
    Image img = random_image(5, 4, 3);
    CHECK(max_difference(flip_horizontal(flip_horizontal(img)), img) == 0.0f);
    CHECK(max_difference(flip_vertical(flip_vertical(img)), img) == 0.0f);
}

TEST(flip_horizontal_mirrors_pixels) {
    Image img(3, 1, 1);
    img.at(0, 0, 0) = 0.1f;
    CHECK_NEAR(flip_horizontal(img).at(2, 0, 0), 0.1, 1e-6);
}

TEST(rotate_swaps_size_and_moves_corner) {
    Image img(4, 2, 1);
    img.at(0, 0, 0) = 1.0f;  // top-left
    Image r = rotate90(img);
    CHECK(r.width() == 2 && r.height() == 4);
    CHECK_NEAR(r.at(1, 0, 0), 1.0, 1e-6);  // top-left goes to top-right
}

TEST(rotating_four_times_gives_original) {
    Image img = random_image(5, 3, 3);
    CHECK(max_difference(rotate90(rotate90(rotate90(rotate90(img)))), img) == 0.0f);
}

TEST(crop_copies_the_right_area) {
    Image img = random_image(6, 6, 1);
    Image c = crop(img, 2, 1, 3, 2);
    CHECK(c.width() == 3 && c.height() == 2);
    CHECK_NEAR(c.at(0, 0, 0), img.at(2, 1, 0), 1e-9);
    CHECK_NEAR(c.at(2, 1, 0), img.at(4, 2, 0), 1e-9);
    CHECK_THROWS(crop(img, 4, 4, 5, 5));
    CHECK_THROWS(crop(img, -1, 0, 2, 2));
}

TEST(nearest_upscale_repeats_pixels) {
    Image img(2, 1, 1);
    img.at(0, 0, 0) = 0.2f;
    img.at(1, 0, 0) = 0.8f;
    Image r = resize_nearest(img, 4, 2);
    CHECK_NEAR(r.at(1, 1, 0), 0.2, 1e-6);
    CHECK_NEAR(r.at(2, 0, 0), 0.8, 1e-6);
}

TEST(bilinear_downscale_averages_2x2_blocks) {
    Image img(4, 4, 1);
    // top-left 2x2 block: 0, 0.2, 0.4, 0.6 -> average 0.3
    img.at(0, 0, 0) = 0.0f;
    img.at(1, 0, 0) = 0.2f;
    img.at(0, 1, 0) = 0.4f;
    img.at(1, 1, 0) = 0.6f;
    Image r = resize_bilinear(img, 2, 2);
    CHECK_NEAR(r.at(0, 0, 0), 0.3, 1e-6);
}

TEST(bilinear_same_size_changes_nothing) {
    Image img = random_image(7, 5, 3);
    CHECK(max_difference(resize_bilinear(img, 7, 5), img) < 1e-6f);
}

TEST(bilinear_keeps_flat_color) {
    Image img(3, 3, 3, 0.45f);
    Image r = resize_bilinear(img, 10, 7);
    for (float v : r.data()) CHECK_NEAR(v, 0.45, 1e-6);
}
