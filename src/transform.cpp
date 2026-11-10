#include <algorithm>
#include <cmath>

#include "imgedit/ops.hpp"

namespace imgedit {

Image flip_horizontal(const Image& img) {
    Image out(img.width(), img.height(), img.channels());
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            for (int c = 0; c < img.channels(); ++c)
                out.at(img.width() - 1 - x, y, c) = img.at(x, y, c);
    return out;
}

Image flip_vertical(const Image& img) {
    Image out(img.width(), img.height(), img.channels());
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            for (int c = 0; c < img.channels(); ++c)
                out.at(x, img.height() - 1 - y, c) = img.at(x, y, c);
    return out;
}

Image rotate90(const Image& img) {
    // Clockwise: pixel (x, y) moves to (height - 1 - y, x), and width/height swap.
    Image out(img.height(), img.width(), img.channels());
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            for (int c = 0; c < img.channels(); ++c)
                out.at(img.height() - 1 - y, x, c) = img.at(x, y, c);
    return out;
}

Image crop(const Image& img, int x0, int y0, int width, int height) {
    if (x0 < 0 || y0 < 0 || width <= 0 || height <= 0 || x0 + width > img.width() ||
        y0 + height > img.height()) {
        throw ImageError("crop area is outside the image");
    }
    Image out(width, height, img.channels());
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            for (int c = 0; c < img.channels(); ++c)
                out.at(x, y, c) = img.at(x0 + x, y0 + y, c);
    return out;
}

Image resize_nearest(const Image& img, int new_width, int new_height) {
    Image out(new_width, new_height, img.channels());
    const float sx = static_cast<float>(img.width()) / new_width;
    const float sy = static_cast<float>(img.height()) / new_height;
    for (int y = 0; y < new_height; ++y)
        for (int x = 0; x < new_width; ++x) {
            int src_x = std::min(static_cast<int>(x * sx), img.width() - 1);
            int src_y = std::min(static_cast<int>(y * sy), img.height() - 1);
            for (int c = 0; c < img.channels(); ++c) out.at(x, y, c) = img.at(src_x, src_y, c);
        }
    return out;
}

Image resize_bilinear(const Image& img, int new_width, int new_height) {
    Image out(new_width, new_height, img.channels());
    const float sx = static_cast<float>(img.width()) / new_width;
    const float sy = static_cast<float>(img.height()) / new_height;

    for (int y = 0; y < new_height; ++y)
        for (int x = 0; x < new_width; ++x) {
            // Map the center of the output pixel back into the source image.
            float fx = (x + 0.5f) * sx - 0.5f;
            float fy = (y + 0.5f) * sy - 0.5f;
            int x0 = static_cast<int>(std::floor(fx));
            int y0 = static_cast<int>(std::floor(fy));
            float tx = fx - x0;  // how far we are between x0 and x0 + 1
            float ty = fy - y0;

            for (int c = 0; c < img.channels(); ++c) {
                // Interpolate along x on the two rows, then between the rows along y.
                float top = (1 - tx) * img.clamped(x0, y0, c) + tx * img.clamped(x0 + 1, y0, c);
                float bottom =
                    (1 - tx) * img.clamped(x0, y0 + 1, c) + tx * img.clamped(x0 + 1, y0 + 1, c);
                out.at(x, y, c) = (1 - ty) * top + ty * bottom;
            }
        }
    return out;
}

}  // namespace imgedit
