#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace imgedit {

class ImageError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// An image stored as floats in [0, 1], channels interleaved (RGBRGB...).
// Working in float avoids rounding errors piling up when filters are chained.
class Image {
public:
    Image() = default;
    Image(int width, int height, int channels, float fill = 0.0f);

    int width() const { return width_; }
    int height() const { return height_; }
    int channels() const { return channels_; }
    bool empty() const { return data_.empty(); }

    float& at(int x, int y, int c) { return data_[index(x, y, c)]; }
    float at(int x, int y, int c) const { return data_[index(x, y, c)]; }

    // Coordinates outside the image are clamped to the nearest edge pixel.
    // Filters use this so they don't need special cases at the borders.
    float clamped(int x, int y, int c) const;

    std::vector<float>& data() { return data_; }
    const std::vector<float>& data() const { return data_; }

private:
    std::size_t index(int x, int y, int c) const {
        return (static_cast<std::size_t>(y) * width_ + x) * channels_ + c;
    }

    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
    std::vector<float> data_;
};

// Load PNG, JPEG, BMP or TGA. Throws ImageError on failure.
Image load_image(const std::string& path);

// Save as PNG or JPEG, picked from the file extension. Throws ImageError on failure.
void save_image(const Image& image, const std::string& path);

}  // namespace imgedit
