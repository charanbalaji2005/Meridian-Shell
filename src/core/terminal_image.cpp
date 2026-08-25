#include "terminal_image.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

namespace meridian::core {

TerminalImage::TerminalImage() = default;

TerminalImage::TerminalImage(int width, int height)
    : width_(width)
    , height_(height)
    , pixels_(width * height, RgbColor{0, 0, 0, 255})
{
}

void TerminalImage::set_pixel(int x, int y, RgbColor color) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[y * width_ + x] = color;
    }
}

RgbColor TerminalImage::get_pixel(int x, int y) const {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return pixels_[y * width_ + x];
    }
    return RgbColor{0, 0, 0, 0};
}

RgbColor TerminalImage::sample_bilinear(float u, float v) const {
    if (width_ <= 0 || height_ <= 0) return RgbColor{0, 0, 0, 255};

    float px = u * (width_ - 1);
    float py = v * (height_ - 1);
    int x0 = std::clamp(static_cast<int>(std::floor(px)), 0, width_ - 1);
    int y0 = std::clamp(static_cast<int>(std::floor(py)), 0, height_ - 1);
    int x1 = std::clamp(x0 + 1, 0, width_ - 1);
    int y1 = std::clamp(y0 + 1, 0, height_ - 1);

    float fx = px - x0;
    float fy = py - y0;

    auto c00 = get_pixel(x0, y0);
    auto c10 = get_pixel(x1, y0);
    auto c01 = get_pixel(x0, y1);
    auto c11 = get_pixel(x1, y1);

    auto interp = [fx, fy](uint8_t v00, uint8_t v10, uint8_t v01, uint8_t v11) -> uint8_t {
        float top = v00 * (1.0f - fx) + v10 * fx;
        float bot = v01 * (1.0f - fx) + v11 * fx;
        return static_cast<uint8_t>(std::clamp(top * (1.0f - fy) + bot * fy, 0.0f, 255.0f));
    };

    return RgbColor{
        interp(c00.r, c10.r, c01.r, c11.r),
        interp(c00.g, c10.g, c01.g, c11.g),
        interp(c00.b, c10.b, c01.b, c11.b),
        interp(c00.a, c10.a, c01.a, c11.a)
    };
}

char TerminalImage::char_for_luminance(float lum, const std::string& charset) const {
    if (charset.empty()) return ' ';
    int idx = static_cast<int>(lum * (charset.size() - 1));
    idx = std::clamp(idx, 0, static_cast<int>(charset.size() - 1));
    return charset[idx];
}

bool TerminalImage::load_file(const std::string& path) {
    if (path.empty()) return false;

    // 1. Try binary Netpbm PPM (P6)
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    std::string magic;
    f >> magic;
    if (magic == "P6") {
        int w = 0, h = 0, maxval = 0;
        f >> w >> h >> maxval;
        f.get(); // skip whitespace
        if (w > 0 && h > 0 && maxval > 0) {
            width_ = w;
            height_ = h;
            pixels_.resize(w * h);
            for (int i = 0; i < w * h; ++i) {
                uint8_t rgb[3];
                f.read(reinterpret_cast<char*>(rgb), 3);
                pixels_[i] = RgbColor{rgb[0], rgb[1], rgb[2], 255};
            }
            return true;
        }
    } else if (magic == "P3") { // Plain text PPM
        int w = 0, h = 0, maxval = 0;
        f >> w >> h >> maxval;
        if (w > 0 && h > 0) {
            width_ = w;
            height_ = h;
            pixels_.resize(w * h);
            for (int i = 0; i < w * h; ++i) {
                int r, g, b;
                f >> r >> g >> b;
                pixels_[i] = RgbColor{static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255};
            }
            return true;
        }
    }

    return false;
}

TerminalImage TerminalImage::create_default_reference_artwork() {
    // Generates a 48x44 pixel art nightscape matching the reference image:
    // Glowing moon, starry sky, clouds, jagged mountains, and reflective lake
    int w = 48;
    int h = 44;
    TerminalImage img(w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / w;
            float ny = static_cast<float>(y) / h;

            // Sky gradient (deep indigo to navy)
            int sky_r = static_cast<int>(12 + ny * 18);
            int sky_g = static_cast<int>(18 + ny * 24);
            int sky_b = static_cast<int>(30 + ny * 35);
            RgbColor col{static_cast<uint8_t>(sky_r), static_cast<uint8_t>(sky_g), static_cast<uint8_t>(sky_b), 255};

            // Glowing Moon at (12, 10), radius ~7.5
            float dx = x - 12.0f;
            float dy = y - 10.0f;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= 7.2f) {
                // Moon surface & craters
                if (dist > 5.8f) {
                    col = RgbColor{255, 245, 185, 255}; // glowing edge
                } else if ((x == 10 && y == 9) || (x == 13 && y == 12) || (x == 9 && y == 11)) {
                    col = RgbColor{225, 215, 155, 255}; // crater
                } else {
                    col = RgbColor{255, 252, 215, 255}; // moon body
                }
            } else if (dist <= 10.5f) {
                // Subtle moon glow halo
                float glow = (10.5f - dist) / 3.3f;
                col.r = static_cast<uint8_t>(std::clamp(col.r + glow * 50.0f, 0.0f, 255.0f));
                col.g = static_cast<uint8_t>(std::clamp(col.g + glow * 50.0f, 0.0f, 255.0f));
                col.b = static_cast<uint8_t>(std::clamp(col.b + glow * 40.0f, 0.0f, 255.0f));
            }

            // Stars
            if ((x == 26 && y == 5) || (x == 38 && y == 7) || (x == 44 && y == 12) ||
                (x == 4 && y == 6) || (x == 33 && y == 10) || (x == 20 && y == 3)) {
                col = RgbColor{240, 248, 255, 255};
            }

            // Clouds across sky (around y = 11 to 16)
            if (y >= 11 && y <= 16 && x >= 18 && x <= 46) {
                float cdist = std::abs(y - 13.5f) + std::abs(x - 32.0f) * 0.2f;
                if (cdist < 3.5f) {
                    col = RgbColor{45, 68, 92, 255};
                } else if (cdist < 4.8f) {
                    col = RgbColor{35, 52, 74, 255};
                }
            }

            // Mountain Peaks (Layer 1 - back mountains)
            float back_mtn = 16.0f + 7.0f * std::sin(nx * 4.5f + 1.2f) + 4.0f * std::cos(nx * 8.0f);
            if (y >= back_mtn && y < 28) {
                // Highlights on left slopes
                if ((x % 6 < 3)) {
                    col = RgbColor{55, 78, 105, 255};
                } else {
                    col = RgbColor{38, 54, 76, 255};
                }
            }

            // Mountain Peaks (Layer 2 - front mountains)
            float mtn_h1 = 18.0f + std::abs(x - 22.0f) * 0.85f;
            float mtn_h2 = 19.0f + std::abs(x - 36.0f) * 0.95f;
            float front_mtn = std::min(mtn_h1, mtn_h2);
            if (y >= front_mtn && y < 30) {
                if ((x >= 16 && x <= 22) || (x >= 30 && x <= 36)) {
                    col = RgbColor{42, 60, 82, 255}; // lit slope
                } else {
                    col = RgbColor{24, 36, 52, 255}; // shadow slope
                }
            }

            // Lake & Water Reflections (y >= 29 to 44)
            if (y >= 29) {
                // Base water color
                col = RgbColor{14, 24, 38, 255};

                // Vertical Moon reflection shimmer in lake (around x = 10 to 16)
                if (x >= 9 && x <= 16) {
                    int dist_from_center = std::abs(x - 12);
                    if ((y % 2 == 0) && (dist_from_center <= ((y - 28) / 3))) {
                        col = RgbColor{255, 242, 175, 255}; // bright shimmer
                    } else if (dist_from_center <= ((y - 28) / 2)) {
                        col = RgbColor{180, 195, 180, 255}; // soft reflection
                    }
                }

                // Pine tree silhouettes along left and right banks
                if (x <= 7 && y < (36 + x)) {
                    col = RgbColor{10, 18, 26, 255}; // left shore pine
                } else if (x >= 40 && y < (34 + (47 - x))) {
                    col = RgbColor{10, 18, 26, 255}; // right shore pine
                }
            }

            img.set_pixel(x, y, col);
        }
    }

    return img;
}

std::vector<std::string> TerminalImage::render_reference_artwork_lines(int max_rows) {
    auto art = create_default_reference_artwork();
    ImageOptions opts;
    opts.mode = ImageRenderMode::HalfBlock;
    opts.target_width = 24;
    opts.target_height = max_rows > 0 ? max_rows : 11;

    std::vector<std::string> lines;
    int target_w = opts.target_width;
    int target_h = opts.target_height;

    for (int r = 0; r < target_h; ++r) {
        std::ostringstream ss;
        for (int c = 0; c < target_w; ++c) {
            float u = static_cast<float>(c) / (target_w - 1);
            float v_top = static_cast<float>(r * 2) / (target_h * 2 - 1);
            float v_bot = static_cast<float>(r * 2 + 1) / (target_h * 2 - 1);

            auto c_top = art.sample_bilinear(u, v_top);
            auto c_bot = art.sample_bilinear(u, v_bot);

            // \033[38;2;R;G;Bm for top, \033[48;2;R;G;Bm for bottom, ▀
            ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
               << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
               << "m▀";
        }
        ss << "\033[0m";
        lines.push_back(ss.str());
    }

    return lines;
}

std::string TerminalImage::render(const ImageOptions& opts) const {
    if (!is_valid()) return "";

    std::ostringstream ss;
    int target_w = std::max(4, opts.target_width);
    int target_h = std::max(2, opts.target_height);

    if (opts.mode == ImageRenderMode::HalfBlock) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v_top = static_cast<float>(r * 2) / (target_h * 2 - 1);
                float v_bot = static_cast<float>(r * 2 + 1) / (target_h * 2 - 1);

                auto c_top = sample_bilinear(u, v_top);
                auto c_bot = sample_bilinear(u, v_bot);

                ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
                   << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
                   << "m▀";
            }
            ss << "\033[0m\n";
        }
    } else if (opts.mode == ImageRenderMode::Ascii) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v = static_cast<float>(r) / (target_h - 1);
                auto col = sample_bilinear(u, v);
                float lum = (0.299f * col.r + 0.587f * col.g + 0.114f * col.b) / 255.0f;
                char ch = char_for_luminance(lum, opts.custom_charset);
                ss << ch;
            }
            ss << "\n";
        }
    } else if (opts.mode == ImageRenderMode::ColorAscii) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v = static_cast<float>(r) / (target_h - 1);
                auto col = sample_bilinear(u, v);
                float lum = (0.299f * col.r + 0.587f * col.g + 0.114f * col.b) / 255.0f;
                char ch = char_for_luminance(lum, opts.custom_charset);
                ss << "\033[38;2;" << static_cast<int>(col.r) << ";" << static_cast<int>(col.g) << ";" << static_cast<int>(col.b) << "m" << ch;
            }
            ss << "\033[0m\n";
        }
    } else if (opts.mode == ImageRenderMode::Hybrid) {
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                float u = static_cast<float>(c) / (target_w - 1);
                float v = static_cast<float>(r) / (target_h - 1);
                auto col = sample_bilinear(u, v);
                float lum = (0.299f * col.r + 0.587f * col.g + 0.114f * col.b) / 255.0f;
                char ch = char_for_luminance(lum, opts.custom_charset);

                ss << "\033[38;2;" << static_cast<int>(col.r) << ";" << static_cast<int>(col.g) << ";" << static_cast<int>(col.b)
                   << ";48;2;" << static_cast<int>(col.r / 3) << ";" << static_cast<int>(col.g / 3) << ";" << static_cast<int>(col.b / 3)
                   << "m" << ch;
            }
            ss << "\033[0m\n";
        }
    }

    return ss.str();
}

} // namespace meridian::core

