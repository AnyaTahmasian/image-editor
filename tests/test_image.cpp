#include <cstdio>
#include <string>

#include "helpers.hpp"
#include "imgedit/image.hpp"
#include "test.hpp"

using namespace imgedit;

TEST(image_is_filled_with_given_value) {
    Image img(4, 3, 3, 0.25f);
    CHECK(img.width() == 4);
    CHECK(img.height() == 3);
    CHECK(img.data().size() == 4u * 3u * 3u);
    CHECK_NEAR(img.at(3, 2, 2), 0.25, 1e-9);
}

TEST(image_rejects_bad_size) {
    CHECK_THROWS(Image(0, 5, 3));
    CHECK_THROWS(Image(5, -1, 3));
    CHECK_THROWS(Image(5, 5, 5));
}

TEST(clamped_returns_edge_pixels_outside_the_image) {
    Image img(2, 2, 1);
    img.at(0, 0, 0) = 0.1f;
    img.at(1, 1, 0) = 0.9f;
    CHECK_NEAR(img.clamped(-5, -5, 0), 0.1, 1e-6);
    CHECK_NEAR(img.clamped(10, 10, 0), 0.9, 1e-6);
}

TEST(png_save_and_load_round_trip) {
    Image img = random_image(7, 5, 3);
    const std::string path = "roundtrip_test.png";
    save_image(img, path);
    Image loaded = load_image(path);
    std::remove(path.c_str());

    CHECK(loaded.width() == 7 && loaded.height() == 5 && loaded.channels() == 3);
    // PNG stores 8 bits per channel, so values can be off by half a step.
    CHECK(max_difference(img, loaded) <= 0.5f / 255.0f + 1e-6f);
}

TEST(load_and_save_report_errors) {
    CHECK_THROWS(load_image("does_not_exist.png"));
    CHECK_THROWS(save_image(Image(2, 2, 3), "out.bmp"));
    CHECK_THROWS(save_image(Image(), "out.png"));
}
