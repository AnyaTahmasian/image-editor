#include "imgedit/image.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace imgedit {

Image::Image(int width, int height, int channels, float fill)
    : width_(width), height_(height), channels_(channels) {
    if (width <= 0 || height <= 0) {
        throw ImageError("image size must be positive");
    }
    if (channels < 1 || channels > 4) {
        throw ImageError("image must have 1 to 4 channels");
    }
    data_.assign(static_cast<std::size_t>(width) * height * channels, fill);
}

float Image::clamped(int x, int y, int c) const {
    x = std::clamp(x, 0, width_ - 1);
    y = std::clamp(y, 0, height_ - 1);
    return at(x, y, c);
}

Image load_image(const std::string& path) {
    int w = 0, h = 0, ch = 0;
    unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!pixels) {
        throw ImageError("could not load '" + path + "': " + stbi_failure_reason());
    }

    Image img(w, h, ch);
    auto& data = img.data();
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = pixels[i] / 255.0f;
    }
    stbi_image_free(pixels);
    return img;
}

static bool ends_with(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin(), [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) == b;
    });
}

void save_image(const Image& image, const std::string& path) {
    if (image.empty()) {
        throw ImageError("cannot save an empty image");
    }

    std::vector<unsigned char> bytes(image.data().size());
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        float v = std::clamp(image.data()[i], 0.0f, 1.0f);
        bytes[i] = static_cast<unsigned char>(std::lround(v * 255.0f));
    }

    const int w = image.width(), h = image.height(), ch = image.channels();
    int ok = 0;
    if (ends_with(path, ".png")) {
        ok = stbi_write_png(path.c_str(), w, h, ch, bytes.data(), w * ch);
    } else if (ends_with(path, ".jpg") || ends_with(path, ".jpeg")) {
        ok = stbi_write_jpg(path.c_str(), w, h, ch, bytes.data(), 92);
    } else {
        throw ImageError("unsupported output format (use .png or .jpg): " + path);
    }
    if (!ok) {
        throw ImageError("could not write '" + path + "'");
    }
}

}  // namespace imgedit
