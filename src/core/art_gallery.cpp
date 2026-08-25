// src/core/art_gallery.cpp
#include "art_gallery.hpp"
#include "graphics/image_decoder.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <random>
#include <sstream>
#include <unistd.h>

namespace meridian::core {

namespace {

RgbColor blend(RgbColor c1, RgbColor c2, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    uint8_t r = static_cast<uint8_t>(c1.r * (1.0f - t) + c2.r * t);
    uint8_t g = static_cast<uint8_t>(c1.g * (1.0f - t) + c2.g * t);
    uint8_t b = static_cast<uint8_t>(c1.b * (1.0f - t) + c2.b * t);
    return RgbColor{r, g, b, 255};
}

// 1. Chainsaw Man: Power & Denji Crimson Slash
TerminalImage make_chainsaw_man_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Dark Charcoal background with subtle gradient
            RgbColor col{static_cast<uint8_t>(18 + ny * 10), static_cast<uint8_t>(20 + ny * 8), static_cast<uint8_t>(26 + ny * 12), 255};

            // Crimson blood-energy slash diagonally across
            float slash_dist = std::abs((nx * 1.2f - ny * 0.9f) - 0.2f);
            if (slash_dist < 0.25f) {
                float intensity = 1.0f - (slash_dist / 0.25f);
                RgbColor slash_col = blend(RgbColor{220, 20, 50, 255}, RgbColor{255, 90, 95, 255}, intensity);
                col = blend(col, slash_col, intensity * 0.85f);
            }

            // Anime character face / silhouette (center-right)
            float cx = nx - 0.55f;
            float cy = ny - 0.45f;
            float dist_face = std::sqrt(cx * cx * 1.5f + cy * cy * 2.0f);

            // Platinum / Blonde Hair
            if (dist_face < 0.38f && ny < 0.65f) {
                float hair_spec = std::sin(nx * 20.0f + ny * 10.0f) * 0.15f;
                RgbColor hair_col = blend(RgbColor{235, 220, 195, 255}, RgbColor{255, 245, 230, 255}, ny + hair_spec);
                col = blend(col, hair_col, 0.90f);
            }

            // Fiend Horns (Crimson Red)
            float h1_x = nx - 0.48f; float h1_y = ny - 0.22f;
            float h2_x = nx - 0.62f; float h2_y = ny - 0.20f;
            if ((std::abs(h1_x + h1_y * 0.4f) < 0.04f && h1_y < 0.12f && h1_y > -0.18f) ||
                (std::abs(h2_x - h2_y * 0.4f) < 0.04f && h2_y < 0.12f && h2_y > -0.18f)) {
                col = RgbColor{185, 28, 28, 255};
            }

            // Glowing Amber Eyes
            if ((std::abs(nx - 0.50f) < 0.035f && std::abs(ny - 0.42f) < 0.025f) ||
                (std::abs(nx - 0.60f) < 0.035f && std::abs(ny - 0.41f) < 0.025f)) {
                col = RgbColor{255, 183, 3, 255};
            }

            // Katana Steel Glint
            float blade_dist = std::abs((nx * 0.8f + ny * 1.1f) - 0.75f);
            if (blade_dist < 0.03f && ny > 0.3f) {
                col = RgbColor{240, 248, 255, 255};
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// 2. Cyberpunk Edgerunners: Neon City Skyline
TerminalImage make_cyberpunk_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Deep Violet to Cyan Sky gradient
            RgbColor sky_top{15, 10, 35, 255};
            RgbColor sky_bot{55, 20, 90, 255};
            RgbColor col = blend(sky_top, sky_bot, ny);

            // Neon Megastructure Silhouettes
            int bldg_idx = static_cast<int>(nx * 8.0f);
            float bldg_h = 0.40f + ((bldg_idx * 7) % 5) * 0.09f;
            if (ny > (1.0f - bldg_h)) {
                col = RgbColor{16, 12, 28, 255};

                // Window lights (Neon Cyan & Electric Yellow)
                if (((x % 3 == 0) && (y % 2 == 0)) && ny > (1.1f - bldg_h)) {
                    col = ((x + y) % 5 == 0) ? RgbColor{0, 240, 255, 255} : RgbColor{254, 228, 64, 255};
                }
            }

            // Holographic Moon / Cyber Ring
            float mx = nx - 0.72f;
            float my = ny - 0.30f;
            float mdist = std::sqrt(mx * mx + my * my);
            if (mdist < 0.16f) {
                col = blend(col, RgbColor{0, 240, 255, 255}, 0.85f);
            } else if (std::abs(mdist - 0.22f) < 0.02f) {
                col = RgbColor{255, 0, 128, 255}; // Neon Pink Ring
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// 3. Synthwave Sunset: Glowing Sun & Horizon Grid
TerminalImage make_synthwave_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Sunset Gradient (Deep Purple -> Hot Pink)
            RgbColor sky_top{38, 12, 60, 255};
            RgbColor sky_bot{180, 25, 110, 255};
            RgbColor col = blend(sky_top, sky_bot, ny * 1.5f);

            // Giant Neon Sun with horizontal slice lines
            float sx = nx - 0.5f;
            float sy = ny - 0.45f;
            float sdist = std::sqrt(sx * sx + sy * sy);
            if (sdist < 0.32f && ny < 0.65f) {
                bool scanline = (y % 3 == 0) && (ny > 0.40f);
                if (!scanline) {
                    float sun_grad = (ny - 0.15f) / 0.50f;
                    col = blend(RgbColor{255, 220, 0, 255}, RgbColor{255, 60, 90, 255}, sun_grad);
                }
            }

            // Retro Horizon Grid
            if (ny >= 0.65f) {
                float grid_y = (ny - 0.65f) / 0.35f;
                col = blend(RgbColor{20, 10, 35, 255}, RgbColor{5, 2, 15, 255}, grid_y);

                // Perspective Grid Lines (Neon Cyan)
                float persp_x = (nx - 0.5f) / (grid_y + 0.15f);
                bool v_line = std::abs(std::fmod(persp_x * 4.0f + 10.0f, 1.0f) - 0.5f) < 0.08f;
                bool h_line = (y == static_cast<int>(h * 0.68f) || y == static_cast<int>(h * 0.74f) ||
                               y == static_cast<int>(h * 0.82f) || y == static_cast<int>(h * 0.92f));

                if (v_line || h_line) {
                    col = RgbColor{0, 245, 212, 255};
                }
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// 4. Studio Ghibli: Anime Meadow & Azure Sky
TerminalImage make_ghibli_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Azure Sky Gradient
            RgbColor sky_top{50, 130, 220, 255};
            RgbColor sky_bot{170, 225, 250, 255};
            RgbColor col = blend(sky_top, sky_bot, ny * 1.6f);

            // Fluffy White Clouds
            float cloud1 = std::sqrt(std::pow(nx - 0.35f, 2) + std::pow((ny - 0.28f) * 1.8f, 2));
            float cloud2 = std::sqrt(std::pow(nx - 0.55f, 2) + std::pow((ny - 0.22f) * 1.8f, 2));
            if (cloud1 < 0.20f || cloud2 < 0.22f) {
                col = blend(col, RgbColor{255, 255, 255, 255}, 0.92f);
            }

            // Rolling Emerald Grass Hills
            float hill1 = 0.60f + std::sin(nx * 4.0f) * 0.08f;
            float hill2 = 0.72f + std::cos(nx * 5.0f) * 0.06f;

            if (ny >= hill2) {
                col = blend(RgbColor{45, 160, 80, 255}, RgbColor{30, 110, 50, 255}, (ny - hill2) * 2.5f);
                // Wildflower sparkles (Pink & Yellow)
                if ((x * 13 + y * 7) % 19 == 0) col = RgbColor{255, 105, 180, 255};
            } else if (ny >= hill1) {
                col = blend(RgbColor{80, 190, 100, 255}, RgbColor{50, 140, 70, 255}, (ny - hill1) * 3.0f);
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

// 5. Deep Space Astronaut & Crescent Galaxy
TerminalImage make_space_art(int w, int h) {
    TerminalImage img(w, h);
    for (int y = 0; y < h; ++y) {
        float ny = static_cast<float>(y) / (h - 1);
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / (w - 1);

            // Deep Obsidian Void
            RgbColor col{10, 8, 20, 255};

            // Cosmic Nebula Swirl (Magenta & Indigo)
            float swirl = std::sin(nx * 3.5f + ny * 2.5f);
            if (swirl > 0.0f) {
                RgbColor neb = blend(RgbColor{120, 30, 180, 255}, RgbColor{240, 60, 140, 255}, swirl);
                col = blend(col, neb, swirl * 0.45f);
            }

            // Glistening Stars
            if ((x * 17 + y * 31) % 43 == 0) {
                col = RgbColor{255, 255, 255, 255};
            }

            // Astronaut Helmet Visor (Gold Reflection)
            float vx = nx - 0.50f;
            float vy = ny - 0.52f;
            float vdist = std::sqrt(vx * vx * 1.2f + vy * vy * 1.6f);
            if (vdist < 0.24f) {
                float spec = std::clamp((0.24f - vdist) / 0.24f, 0.0f, 1.0f);
                col = blend(RgbColor{245, 180, 25, 255}, RgbColor{255, 235, 140, 255}, spec);
            } else if (vdist < 0.30f) {
                col = RgbColor{220, 225, 235, 255}; // Suit White
            }

            img.set_pixel(x, y, col);
        }
    }
    return img;
}

} // namespace

size_t ArtGallery::theme_count() {
    return 5;
}

ArtworkTheme ArtGallery::get_artwork_by_index(size_t index, int width, int height) {
    // 1. First check if user configured a custom single artwork
    const char* env_art = std::getenv("MERIDIAN_ARTWORK");
    if (env_art && access(env_art, R_OK) == 0) {
        TerminalImage custom_img;
        if (custom_img.load_file(env_art)) {
            return ArtworkTheme{"custom", "Custom Artwork", custom_img};
        }
    }

    const char* home = std::getenv("HOME");
    if (home) {
        std::string p1 = std::string(home) + "/.config/meridian/artwork.jpg";
        std::string p2 = std::string(home) + "/.config/meridian/artwork.png";
        TerminalImage custom_img;
        if (custom_img.load_file(p1) || custom_img.load_file(p2)) {
            return ArtworkTheme{"user_default", "User Artwork", custom_img};
        }
    }

    // 2. Select from curated artwork gallery
    size_t theme_id = index % theme_count();

    switch (theme_id) {
        case 0: return ArtworkTheme{"chainsaw_man", "Chainsaw Man", make_chainsaw_man_art(width, height)};
        case 1: return ArtworkTheme{"cyberpunk", "Cyberpunk Night", make_cyberpunk_art(width, height)};
        case 2: return ArtworkTheme{"synthwave", "Synthwave Horizon", make_synthwave_art(width, height)};
        case 3: return ArtworkTheme{"ghibli", "Anime Meadow", make_ghibli_art(width, height)};
        case 4: default: return ArtworkTheme{"space", "Cosmic Voyage", make_space_art(width, height)};
    }
}

ArtworkTheme ArtGallery::get_next_artwork(int width, int height) {
    // Use session/time-based random seed to rotate artwork on each terminal open
    static size_t session_counter = 0;
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    size_t pick = (static_cast<size_t>(now) + getpid() + (session_counter++)) % theme_count();
    return get_artwork_by_index(pick, width, height);
}

std::vector<std::string> ArtGallery::render_artwork_lines(const TerminalImage& img, int target_cols, int target_rows) {
    std::vector<std::string> lines;
    int src_w = img.width();
    int src_h = img.height();
    if (src_w <= 0 || src_h <= 0) return lines;

    for (int r = 0; r < target_rows; ++r) {
        std::ostringstream ss;
        for (int c = 0; c < target_cols; ++c) {
            float u = static_cast<float>(c) / std::max(1, target_cols - 1);
            float v_top = static_cast<float>(r * 2) / std::max(1, target_rows * 2 - 1);
            float v_bot = static_cast<float>(r * 2 + 1) / std::max(1, target_rows * 2 - 1);

            int x = std::clamp(static_cast<int>(u * (src_w - 1)), 0, src_w - 1);
            int y_top = std::clamp(static_cast<int>(v_top * (src_h - 1)), 0, src_h - 1);
            int y_bot = std::clamp(static_cast<int>(v_bot * (src_h - 1)), 0, src_h - 1);

            RgbColor c_top = img.get_pixel(x, y_top);
            RgbColor c_bot = img.get_pixel(x, y_bot);

            // TrueColor 24-bit half-block character
            ss << "\033[38;2;" << static_cast<int>(c_top.r) << ";" << static_cast<int>(c_top.g) << ";" << static_cast<int>(c_top.b)
               << ";48;2;" << static_cast<int>(c_bot.r) << ";" << static_cast<int>(c_bot.g) << ";" << static_cast<int>(c_bot.b)
               << "m▀";
        }
        ss << "\033[0m";
        lines.push_back(ss.str());
    }

    return lines;
}

} // namespace meridian::core
