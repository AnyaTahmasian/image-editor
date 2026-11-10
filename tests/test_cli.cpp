#include <cstdio>

#include "helpers.hpp"
#include "imgedit/cli.hpp"
#include "test.hpp"

using namespace imgedit;

TEST(operations_are_parsed) {
    Image img = random_image(8, 6, 3);
    CHECK(apply_operation(img, "grayscale").channels() == 1);
    CHECK(apply_operation(img, "rotate").width() == 6);
    Image r = apply_operation(img, "resize=4x3");
    CHECK(r.width() == 4 && r.height() == 3);
    Image c = apply_operation(img, "crop=1,1,2,3");
    CHECK(c.width() == 2 && c.height() == 3);
}

TEST(bad_operations_are_rejected) {
    Image img(4, 4, 3);
    CHECK_THROWS(apply_operation(img, "unknown"));
    CHECK_THROWS(apply_operation(img, "blur"));         // missing value
    CHECK_THROWS(apply_operation(img, "blur=abc"));     // not a number
    CHECK_THROWS(apply_operation(img, "blur=2px"));     // trailing junk
    CHECK_THROWS(apply_operation(img, "resize=10"));    // needs WxH
    CHECK_THROWS(apply_operation(img, "resize=0x5"));   // zero size
    CHECK_THROWS(apply_operation(img, "box-blur=1.5")); // must be whole
    CHECK_THROWS(apply_operation(img, "invert=1"));     // takes no value
}

TEST(cli_runs_a_pipeline_end_to_end) {
    save_image(random_image(16, 16, 3), "cli_in.png");
    int code = run_cli({"cli_in.png", "cli_out.png", "resize=8x8", "blur=1", "grayscale"});
    CHECK(code == 0);
    Image out = load_image("cli_out.png");
    CHECK(out.width() == 8 && out.channels() == 1);
    std::remove("cli_in.png");
    std::remove("cli_out.png");
}

TEST(cli_returns_error_code) {
    CHECK(run_cli({}) == 1);
    CHECK(run_cli({"missing.png", "out.png"}) == 1);
}
