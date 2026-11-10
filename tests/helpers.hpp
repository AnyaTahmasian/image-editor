#pragma once

#include <random>

#include "imgedit/image.hpp"

// Random image with a fixed seed so tests are repeatable.
inline imgedit::Image random_image(int w, int h, int channels, unsigned seed = 1) {
    imgedit::Image img(w, h, channels);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (float& v : img.data()) v = dist(rng);
    return img;
}

inline float max_difference(const imgedit::Image& a, const imgedit::Image& b) {
    if (a.width() != b.width() || a.height() != b.height() || a.channels() != b.channels())
        return 1e9f;
    float m = 0.0f;
    for (std::size_t i = 0; i < a.data().size(); ++i) {
        float d = a.data()[i] - b.data()[i];
        if (d < 0) d = -d;
        if (d > m) m = d;
    }
    return m;
}
