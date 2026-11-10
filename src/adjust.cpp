#include <algorithm>
#include <cmath>

#include "imgedit/ops.hpp"

namespace imgedit {

namespace {

// Rec. 601 luma weights: the eye is most sensitive to green, least to blue.
float luminance(const Image& img, int x, int y) {
    if (img.channels() < 3) return img.at(x, y, 0);
    return 0.299f * img.at(x, y, 0) + 0.587f * img.at(x, y, 1) + 0.114f * img.at(x, y, 2);
}

bool is_alpha(const Image& img, int c) {
    return (img.channels() == 4 && c == 3) || (img.channels() == 2 && c == 1);
}

// Applies f to every color channel and leaves alpha alone.
template <typename F>
Image map_colors(const Image& img, F f) {
    Image out = img;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            for (int c = 0; c < img.channels(); ++c)
                if (!is_alpha(img, c)) out.at(x, y, c) = f(img.at(x, y, c));
    return out;
}

}  // namespace

Image grayscale(const Image& img) {
    Image out(img.width(), img.height(), 1);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            out.at(x, y, 0) = luminance(img, x, y);
    return out;
}

Image brightness(const Image& img, float amount) {
    return map_colors(img, [amount](float v) { return std::clamp(v + amount, 0.0f, 1.0f); });
}

Image contrast(const Image& img, float factor) {
    // Stretch values away from (or toward) middle gray.
    return map_colors(img, [factor](float v) {
        return std::clamp((v - 0.5f) * factor + 0.5f, 0.0f, 1.0f);
    });
}

Image invert(const Image& img) {
    return map_colors(img, [](float v) { return 1.0f - v; });
}

Image threshold(const Image& img, float level) {
    Image gray = grayscale(img);
    for (float& v : gray.data()) v = v >= level ? 1.0f : 0.0f;
    return gray;
}

std::array<int, 256> histogram(const Image& img) {
    std::array<int, 256> hist{};
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            int bin = static_cast<int>(std::lround(std::clamp(luminance(img, x, y), 0.0f, 1.0f) * 255));
            ++hist[bin];
        }
    return hist;
}

Image equalize_histogram(const Image& img) {
    // Build the cumulative distribution of brightness values, then map each
    // level to its position in that distribution. Levels that many pixels share
    // get spread apart, which raises contrast in dull or washed-out photos.
    auto hist = histogram(img);
    std::array<float, 256> cdf{};
    int running = 0;
    for (int i = 0; i < 256; ++i) {
        running += hist[i];
        cdf[i] = static_cast<float>(running);
    }

    const float total = static_cast<float>(img.width()) * img.height();
    const float cdf_min = *std::find_if(cdf.begin(), cdf.end(), [](float v) { return v > 0; });
    if (total == cdf_min) return img;  // a single flat color: nothing to spread

    std::array<float, 256> lut{};
    for (int i = 0; i < 256; ++i) {
        lut[i] = std::max(0.0f, (cdf[i] - cdf_min) / (total - cdf_min));
    }

    // Scale each pixel's color by how much its brightness changed, so hues stay the same.
    Image out = img;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            float lum = std::clamp(luminance(img, x, y), 0.0f, 1.0f);
            float target = lut[static_cast<int>(std::lround(lum * 255))];
            for (int c = 0; c < img.channels(); ++c) {
                if (is_alpha(img, c)) continue;
                if (img.channels() < 3) {
                    out.at(x, y, c) = target;
                } else {
                    float scale = lum > 0.0f ? target / lum : 0.0f;
                    out.at(x, y, c) = lum > 0.0f ? std::clamp(img.at(x, y, c) * scale, 0.0f, 1.0f) : target;
                }
            }
        }
    return out;
}

}  // namespace imgedit
