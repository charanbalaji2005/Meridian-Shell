#pragma once
// zephyr-config / theme.hpp
//
// Terminal color schemes: the 16 ANSI colors plus background, foreground,
// cursor and selection. Pure data (no Qt) so it is unit-testable and the
// GUI simply converts each Rgb to a QColor.

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace zephyr::config {

struct Rgb {
    uint8_t r = 0, g = 0, b = 0;
    bool operator==(const Rgb& o) const { return r == o.r && g == o.g && b == o.b; }
};

// Accepts "#RRGGBB" or "RRGGBB" (case-insensitive).
std::optional<Rgb> parse_hex_color(const std::string& text);
std::string to_hex_color(const Rgb& c); // "#rrggbb"

struct Theme {
    std::string id;           // stable key stored in the config file
    std::string display_name;
    bool dark = true;
    Rgb background;
    Rgb foreground;
    Rgb cursor;
    Rgb selection;
    std::array<Rgb, 16> ansi{};
};

const std::vector<Theme>& builtin_themes();
const Theme* find_theme(const std::string& id);
// The theme to use for a given id, falling back to the default dark/light
// theme when the id is unknown.
const Theme& theme_or_default(const std::string& id, bool prefer_dark = true);

} // namespace zephyr::config
