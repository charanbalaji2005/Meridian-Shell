#include "terminal_image.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

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

#include "chainsaw_man_data.inl"

TerminalImage TerminalImage::create_default_reference_artwork() {
    // 1. Try loading user custom artwork or bundled artwork from file
    const char* env_art = std::getenv("MERIDIAN_ARTWORK");
    if (env_art && access(env_art, R_OK) == 0) {
        TerminalImage img;
        if (img.load_file(env_art)) return img;
    }

    const char* home = std::getenv("HOME");
    if (home) {
        std::string p1 = std::string(home) + "/.config/meridian/artwork.jpg";
        std::string p2 = std::string(home) + "/.config/meridian/artwork.ppm";
        TerminalImage img;
        if (img.load_file(p1) || img.load_file(p2)) return img;
    }

    std::string candidate_paths[] = {
        "resources/images/artwork.jpg",
        "resources/images/artwork_thumb.ppm",
        "resources/images/artwork.png"
    };
    for (const auto& cp : candidate_paths) {
        TerminalImage img;
        if (img.load_file(cp)) return img;
    }

    // 2. Fallback to embedded Chainsaw Man pixel buffer
    int w = 48;
    int h = 44;
    TerminalImage img(w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 3;
            uint8_t r = kChainsawManPixels[idx];
            uint8_t g = kChainsawManPixels[idx + 1];
            uint8_t b = kChainsawManPixels[idx + 2];
            img.set_pixel(x, y, RgbColor{r, g, b, 255});
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

static std::string to_base64(const uint8_t* data, size_t len) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t b = (data[i] << 16) | ((i + 1 < len ? data[i + 1] : 0) << 8) | (i + 2 < len ? data[i + 2] : 0);
        out.push_back(table[(b >> 18) & 0x3F]);
        out.push_back(table[(b >> 12) & 0x3F]);
        out.push_back((i + 1 < len) ? table[(b >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < len) ? table[b & 0x3F] : '=');
    }
    return out;
}

std::string TerminalImage::render(const ImageOptions& opts) const {
    if (!is_valid()) return "";

    std::ostringstream ss;
    int target_w = std::max(4, opts.target_width);
    int target_h = std::max(2, opts.target_height);

    if (opts.mode == ImageRenderMode::RealRaster) {
        std::vector<uint8_t> raw_rgba(width_ * height_ * 4);
        for (int i = 0; i < width_ * height_; ++i) {
            raw_rgba[i * 4 + 0] = pixels_[i].r;
            raw_rgba[i * 4 + 1] = pixels_[i].g;
            raw_rgba[i * 4 + 2] = pixels_[i].b;
            raw_rgba[i * 4 + 3] = pixels_[i].a;
        }
        std::string b64 = to_base64(raw_rgba.data(), raw_rgba.size());
        ss << "\033_Ga=T,f=32,s=" << width_ << ",v=" << height_
           << ",c=" << target_w << ",r=" << target_h << ";" << b64 << "\033\\";
        return ss.str();
    } else if (opts.mode == ImageRenderMode::Pixel) {
        // Nearest-neighbor pixel art mode
        for (int r = 0; r < target_h; ++r) {
            for (int c = 0; c < target_w; ++c) {
                int px = std::clamp(c * width_ / target_w, 0, width_ - 1);
                int py_top = std::clamp((r * 2) * height_ / (target_h * 2), 0, height_ - 1);
                int py_bot = std::clamp((r * 2 + 1) * height_ / (target_h * 2), 0, height_ - 1);

                auto c_top = get_pixel(px, py_top);
                auto c_bot = get_pixel(px, py_bot);

                ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
                   << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
                   << "m▀";
            }
            ss << "\033[0m\n";
        }
    } else if (opts.mode == ImageRenderMode::HalfBlock) {
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

std::string TerminalImage::render_kitty_graphics_artwork(int col, int row, int cols_spanned, int rows_spanned) {
    std::ostringstream ss;
    ss << "\033_Ga=T,f=100,t=d,c=" << cols_spanned << ",r=" << rows_spanned
       << ",i=1,X=" << col << ",Y=" << row << ";" << kChainsawManKittyBase64 << "\033\\";
    return ss.str();
}

} // namespace meridian::core

