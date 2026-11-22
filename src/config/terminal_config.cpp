// zephyr-config / terminal_config.cpp
#include "terminal_config.hpp"
#include <cstdlib>

namespace zephyr::config {

ColorPalette ColorPalette::vivid_neon() {
    ColorPalette p;
    p.background = "#090B10";
    p.foreground = "#F5F7FA";
    p.neon_cyan = "#00E5FF";
    p.electric_blue = "#3B82F6";
    p.neon_purple = "#A855F7";
    p.hot_pink = "#F43F5E";
    p.neon_green = "#22C55E";
    p.amber = "#F59E0B";
    p.yellow = "#FDE047";
    p.orange = "#FB923C";
    return p;
}

ColorPalette ColorPalette::dark_nord() {
    ColorPalette p;
    p.background = "#2e3440";
    p.foreground = "#d8dee9";
    p.neon_cyan = "#88c0d0";
    p.electric_blue = "#81a1c1";
    p.neon_purple = "#b48ead";
    p.hot_pink = "#bf616a";
    p.neon_green = "#a3be8c";
    p.amber = "#ebcb8b";
    p.yellow = "#ebcb8b";
    p.orange = "#d08770";
    return p;
}

TerminalConfig::TerminalConfig(std::string path) : config_(std::move(path)) {}

namespace {

// One row per persisted setting: key name + pointer-to-member. Load and
// save both walk this table, so adding a setting is a one-line change and
// the two directions can never drift apart.
struct IntField { const char* key; int TerminalSettings::*member; };
struct BoolField { const char* key; bool TerminalSettings::*member; };
struct StrField { const char* key; std::string TerminalSettings::*member; };

const IntField kIntFields[] = {
    {"scrollback_lines", &TerminalSettings::scrollback_lines},
    {"scroll_multiplier", &TerminalSettings::scroll_multiplier},
    {"font_size", &TerminalSettings::font_size},
    {"line_height_percent", &TerminalSettings::line_height_percent},
    {"window_opacity", &TerminalSettings::window_opacity},
    {"background_image_opacity", &TerminalSettings::background_image_opacity},
    {"padding", &TerminalSettings::padding},
    {"corner_radius", &TerminalSettings::corner_radius},
    {"max_fps", &TerminalSettings::max_fps},
};
const BoolField kBoolFields[] = {
    {"cursor_blink", &TerminalSettings::cursor_blink},
    {"copy_on_select", &TerminalSettings::copy_on_select},
    {"confirm_close_running", &TerminalSettings::confirm_close_running},
    {"warn_multiline_paste", &TerminalSettings::warn_multiline_paste},
    {"font_ligatures", &TerminalSettings::font_ligatures},
    {"tab_bar_visible", &TerminalSettings::tab_bar_visible},
    {"sidebar_visible", &TerminalSettings::sidebar_visible},
    {"animations_enabled", &TerminalSettings::animations_enabled},
    {"reduced_motion", &TerminalSettings::reduced_motion},
    {"restore_session", &TerminalSettings::restore_session},
    {"glow_border", &TerminalSettings::glow_border},
    {"frameless_window", &TerminalSettings::frameless_window},
    {"minimal_chrome", &TerminalSettings::minimal_chrome},
};
const StrField kStrFields[] = {
    {"default_shell", &TerminalSettings::default_shell},
    {"startup_directory", &TerminalSettings::startup_directory},
    {"cursor_style", &TerminalSettings::cursor_style},
    {"font_family", &TerminalSettings::font_family},
    {"theme_name", &TerminalSettings::theme_name},
    {"ui_mode", &TerminalSettings::ui_mode},
    {"accent_color", &TerminalSettings::accent_color},
    {"background_image", &TerminalSettings::background_image},
    {"default_profile", &TerminalSettings::default_profile},
    {"glow_color", &TerminalSettings::glow_color},
    {"glow_color2", &TerminalSettings::glow_color2},
};

int clamp_int(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

bool is_hex_color(const std::string& s) {
    if (s.size() != 7 || s[0] != '#') return false;
    for (std::size_t i = 1; i < s.size(); ++i) {
        char c = s[i];
        bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        if (!hex) return false;
    }
    return true;
}

} // namespace

void TerminalSettings::sanitize() {
    const TerminalSettings defaults;
    scrollback_lines = clamp_int(scrollback_lines, 0, 1000000);
    scroll_multiplier = clamp_int(scroll_multiplier, 1, 20);
    font_size = clamp_int(font_size, 6, 72);
    line_height_percent = clamp_int(line_height_percent, 80, 200);
    window_opacity = clamp_int(window_opacity, 30, 100);
    background_image_opacity = clamp_int(background_image_opacity, 0, 100);
    padding = clamp_int(padding, 0, 64);
    corner_radius = clamp_int(corner_radius, 0, 32);
    if (!is_hex_color(glow_color)) glow_color = defaults.glow_color;
    if (!is_hex_color(glow_color2)) glow_color2 = defaults.glow_color2;
    if (max_fps != 30 && max_fps != 60 && max_fps != 120) max_fps = defaults.max_fps;
    if (cursor_style != "block" && cursor_style != "underline" && cursor_style != "bar") {
        cursor_style = defaults.cursor_style;
    }
    if (ui_mode != "dark" && ui_mode != "light" && ui_mode != "system") ui_mode = defaults.ui_mode;
    if (!is_hex_color(accent_color)) accent_color = defaults.accent_color;
    if (font_family.empty()) font_family = defaults.font_family;
    if (default_profile.empty()) default_profile = defaults.default_profile;
}

TerminalSettings neon_night_preset(TerminalSettings s) {
    s.theme_name = "neon-night";
    s.ui_mode = "dark";
    s.accent_color = "#ff6ad5";
    s.window_opacity = 82;
    s.glow_border = true;
    s.glow_color = "#ff5fd2";
    s.glow_color2 = "#8b7cf6";
    s.corner_radius = 14;
    s.minimal_chrome = true;
    s.sidebar_visible = false;
    s.padding = 12;
    s.sanitize();
    return s;
}

void TerminalConfig::settings_from_config() {
    const TerminalSettings defaults;
    for (const auto& f : kIntFields) {
        std::string raw = config_.get(f.key, std::to_string(defaults.*f.member));
        char* end = nullptr;
        long v = std::strtol(raw.c_str(), &end, 10);
        settings_.*f.member = (end && end != raw.c_str()) ? static_cast<int>(v) : defaults.*f.member;
    }
    for (const auto& f : kBoolFields) settings_.*f.member = config_.get_bool(f.key, defaults.*f.member);
    for (const auto& f : kStrFields) settings_.*f.member = config_.get(f.key, defaults.*f.member);
    settings_.sanitize();

    settings_.palette = settings_.theme_name == "dark-nord" ? ColorPalette::dark_nord()
                                                            : ColorPalette::vivid_neon();
}

void TerminalConfig::settings_to_config() {
    settings_.sanitize();
    for (const auto& f : kIntFields) config_.set(f.key, std::to_string(settings_.*f.member));
    for (const auto& f : kBoolFields) config_.set_bool(f.key, settings_.*f.member);
    for (const auto& f : kStrFields) config_.set(f.key, settings_.*f.member);
}

bool TerminalConfig::load() {
    bool existed = config_.load();
    settings_from_config();
    return existed;
}

bool TerminalConfig::save() {
    settings_to_config();
    return config_.save();
}

} // namespace zephyr::config
