#include "imgedit/cli.hpp"

#include <iostream>
#include <sstream>

#include "imgedit/ops.hpp"

namespace imgedit {

namespace {

const char* kUsage = R"(usage: imgedit <input> <output> [operation ...]

Operations are applied left to right:
  grayscale               invert                  equalize
  flip-h                  flip-v                  rotate        (90 deg clockwise)
  edges                   (Sobel edge detection)
  blur=SIGMA              gaussian blur, e.g. blur=2
  box-blur=RADIUS         e.g. box-blur=3
  sharpen=AMOUNT          e.g. sharpen=1.5
  brightness=DELTA        -1 to 1, e.g. brightness=0.1
  contrast=FACTOR         1 = unchanged, e.g. contrast=1.3
  threshold=LEVEL         0 to 1, e.g. threshold=0.5
  resize=WxH              bilinear, e.g. resize=800x600
  resize-nearest=WxH
  crop=X,Y,W,H

example: imgedit photo.jpg out.png resize=640x480 sharpen=1 contrast=1.2
)";

float parse_float(const std::string& name, const std::string& value) {
    std::size_t used = 0;
    float result = 0.0f;
    try {
        result = std::stof(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (value.empty() || used != value.size()) {
        throw ImageError("'" + name + "' needs a number, got '" + value + "'");
    }
    return result;
}

int parse_int(const std::string& name, const std::string& value) {
    float f = parse_float(name, value);
    if (f != static_cast<int>(f)) {
        throw ImageError("'" + name + "' needs a whole number, got '" + value + "'");
    }
    return static_cast<int>(f);
}

// Splits "a,b,c" (or "WxH" with sep = 'x') into ints.
std::vector<int> parse_ints(const std::string& name, const std::string& value, char sep,
                            std::size_t count) {
    std::vector<int> result;
    std::stringstream ss(value);
    std::string part;
    while (std::getline(ss, part, sep)) result.push_back(parse_int(name, part));
    if (result.size() != count) {
        throw ImageError("'" + name + "' has the wrong format: '" + value + "'");
    }
    return result;
}

}  // namespace

Image apply_operation(const Image& img, const std::string& op) {
    const auto eq = op.find('=');
    const std::string name = op.substr(0, eq);
    const std::string value = eq == std::string::npos ? "" : op.substr(eq + 1);
    const bool has_value = eq != std::string::npos;

    // operations without a value
    if (!has_value) {
        if (name == "grayscale") return grayscale(img);
        if (name == "invert") return invert(img);
        if (name == "equalize") return equalize_histogram(img);
        if (name == "flip-h") return flip_horizontal(img);
        if (name == "flip-v") return flip_vertical(img);
        if (name == "rotate") return rotate90(img);
        if (name == "edges") return sobel_edges(img);
    }

    // operations with a value
    if (has_value) {
        if (name == "blur") return gaussian_blur(img, parse_float(name, value));
        if (name == "box-blur") return box_blur(img, parse_int(name, value));
        if (name == "sharpen") return sharpen(img, parse_float(name, value));
        if (name == "brightness") return brightness(img, parse_float(name, value));
        if (name == "contrast") return contrast(img, parse_float(name, value));
        if (name == "threshold") return threshold(img, parse_float(name, value));
        if (name == "resize" || name == "resize-nearest") {
            auto size = parse_ints(name, value, 'x', 2);
            if (size[0] <= 0 || size[1] <= 0) throw ImageError("resize size must be positive");
            return name == "resize" ? resize_bilinear(img, size[0], size[1])
                                    : resize_nearest(img, size[0], size[1]);
        }
        if (name == "crop") {
            auto r = parse_ints(name, value, ',', 4);
            return crop(img, r[0], r[1], r[2], r[3]);
        }
    }

    throw ImageError("unknown operation '" + op + "' (run imgedit --help)");
}

int run_cli(const std::vector<std::string>& args) {
    if (args.size() == 1 && (args[0] == "-h" || args[0] == "--help")) {
        std::cout << kUsage;
        return 0;
    }
    if (args.size() < 2) {
        std::cerr << kUsage;
        return 1;
    }

    try {
        Image img = load_image(args[0]);
        for (std::size_t i = 2; i < args.size(); ++i) {
            img = apply_operation(img, args[i]);
        }
        save_image(img, args[1]);
        std::cout << "saved " << args[1] << " (" << img.width() << "x" << img.height() << ")\n";
    } catch (const ImageError& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

}  // namespace imgedit
