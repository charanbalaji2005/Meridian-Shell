// zephyr-config / theme.cpp
#include "theme.hpp"

#include <cstdio>

namespace zephyr::config {

namespace {

int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

Rgb h(const char* hex) { return *parse_hex_color(hex); }

Theme make(const char* id, const char* name, bool dark, const char* bg, const char* fg,
           const char* cursor, const char* selection, const std::array<const char*, 16>& ansi) {
    Theme t;
    t.id = id;
    t.display_name = name;
    t.dark = dark;
    t.background = h(bg);
    t.foreground = h(fg);
    t.cursor = h(cursor);
    t.selection = h(selection);
    for (std::size_t i = 0; i < 16; ++i) t.ansi[i] = h(ansi[i]);
    return t;
}

} // namespace

std::optional<Rgb> parse_hex_color(const std::string& text) {
    std::string s = (!text.empty() && text[0] == '#') ? text.substr(1) : text;
    if (s.size() != 6) return std::nullopt;
    int v[6];
    for (int i = 0; i < 6; ++i) {
        v[i] = hex_value(s[static_cast<std::size_t>(i)]);
        if (v[i] < 0) return std::nullopt;
    }
    return Rgb{static_cast<uint8_t>(v[0] * 16 + v[1]), static_cast<uint8_t>(v[2] * 16 + v[3]),
               static_cast<uint8_t>(v[4] * 16 + v[5])};
}

std::string to_hex_color(const Rgb& c) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", c.r, c.g, c.b);
    return buf;
}

const std::vector<Theme>& builtin_themes() {
    static const std::vector<Theme> themes = {
        make("zephyr-dark", "Zephyr Dark", true, "#0f1218", "#dce3ee", "#8b7cf6", "#3a3566",
             {"#1b2029", "#f0717d", "#7fd49a", "#ebc46f", "#6fa8f5", "#b98cf2", "#5fd1d9", "#c5cdd9",
              "#4c5566", "#ff8a94", "#9be3b1", "#f5d68f", "#91bdfa", "#cda8f7", "#85e0e6", "#f2f5fa"}),
        make("zephyr-light", "Zephyr Light", false, "#f7f8fa", "#2a2f3a", "#6d5bd0", "#d8d2f7",
             {"#2a2f3a", "#c9364a", "#2e8b57", "#a86a00", "#2f66c9", "#8a3fc2", "#1e8c96", "#8a93a3",
              "#5c6577", "#e04b5e", "#3ba36a", "#c28400", "#4a7fe0", "#a55dd6", "#2aa5b0", "#d9dee7"}),
        // The palette Zephyr shipped with before the redesign, kept for users who like it.
        make("vivid-neon", "Vivid Neon", true, "#090b10", "#f5f7fa", "#00e5ff", "#1d3a66",
             {"#15171a", "#ef4444", "#22c55e", "#fde047", "#3b82f6", "#a855f7", "#00e5ff", "#f5f7fa",
              "#5c6370", "#f43f5e", "#4ade80", "#fef08a", "#60a5fa", "#c084fc", "#22d3ee", "#ffffff"}),
        // Deep purple-navy with neon pink accents: pairs with glow_border.
        make("neon-night", "Neon Night", true, "#16131f", "#e8e3f5", "#ff6ad5", "#4a2b5c",
             {"#241f33", "#ff5c8a", "#7ee787", "#ffd479", "#7aa2ff", "#ff6ad5", "#6ee7f0", "#d8d2ea",
              "#5b5470", "#ff7aa2", "#a4f3a8", "#ffe29a", "#9db8ff", "#ff8ee0", "#96f0f6", "#ffffff"}),
        make("dark-nord", "Nord", true, "#2e3440", "#d8dee9", "#d8dee9", "#434c5e",
             {"#3b4252", "#bf616a", "#a3be8c", "#ebcb8b", "#81a1c1", "#b48ead", "#88c0d0", "#e5e9f0",
              "#4c566a", "#bf616a", "#a3be8c", "#ebcb8b", "#81a1c1", "#b48ead", "#8fbcbb", "#eceff4"}),
        make("gruvbox-dark", "Gruvbox Dark", true, "#282828", "#ebdbb2", "#ebdbb2", "#504945",
             {"#282828", "#cc241d", "#98971a", "#d79921", "#458588", "#b16286", "#689d6a", "#a89984",
              "#928374", "#fb4934", "#b8bb26", "#fabd2f", "#83a598", "#d3869b", "#8ec07c", "#ebdbb2"}),
        make("solarized-light", "Solarized Light", false, "#fdf6e3", "#657b83", "#586e75", "#eee8d5",
             {"#073642", "#dc322f", "#859900", "#b58900", "#268bd2", "#d33682", "#2aa198", "#eee8d5",
              "#002b36", "#cb4b16", "#586e75", "#657b83", "#839496", "#6c71c4", "#93a1a1", "#fdf6e3"}),
    };
    return themes;
}

const Theme* find_theme(const std::string& id) {
    for (const auto& t : builtin_themes()) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

const Theme& theme_or_default(const std::string& id, bool prefer_dark) {
    if (const Theme* t = find_theme(id)) return *t;
    return *find_theme(prefer_dark ? "zephyr-dark" : "zephyr-light");
}

} // namespace zephyr::config
