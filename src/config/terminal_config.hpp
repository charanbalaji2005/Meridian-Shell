#pragma once
// zephyr-config / terminal_config.hpp
//
// Spec §75's terminal.toml settings with vivid neon color palette definitions,
// scrollback size, default shell, font, cursor style, and theme management.

#include "../core/config.hpp"
#include <string>

namespace zephyr::config {

struct ColorPalette {
    std::string background = "#090B10";
    std::string foreground = "#F5F7FA";
    std::string neon_cyan = "#00E5FF";
    std::string electric_blue = "#3B82F6";
    std::string neon_purple = "#A855F7";
    std::string hot_pink = "#F43F5E";
    std::string neon_green = "#22C55E";
    std::string amber = "#F59E0B";
    std::string yellow = "#FDE047";
    std::string orange = "#FB923C";

    static ColorPalette vivid_neon();
    static ColorPalette dark_nord();
};

struct TerminalSettings {
    // --- Terminal behaviour ---
    int scrollback_lines = 10000;      // ScreenBuffer scrollback limit (0 disables history)
    std::string default_shell = "/bin/bash"; // legacy: used by the CLI; the GUI uses profiles
    std::string startup_directory;      // empty = home directory
    std::string cursor_style = "block"; // block | underline | bar
    bool cursor_blink = true;
    bool copy_on_select = false;
    bool confirm_close_running = true;  // ask before closing a pane whose shell has children
    bool warn_multiline_paste = true;   // confirm multi-line paste when bracketed paste is off
    int scroll_multiplier = 3;          // lines per mouse-wheel notch

    // --- Fonts ---
    std::string font_family = "monospace";
    int font_size = 13;
    bool font_ligatures = true;
    int line_height_percent = 100;      // 80..200

    // --- Appearance ---
    // A builtin theme id (config/theme.hpp) or "zephyr-auto", which picks
    // Zephyr Dark/Light to match the light/dark mode of the window chrome.
    std::string theme_name = "zephyr-auto";
    std::string ui_mode = "dark";       // dark | light | system (chrome follows this)
    std::string accent_color = "#8b7cf6";
    int window_opacity = 100;           // percent, 30..100 (needs a compositor)
    std::string background_image;       // optional image path drawn behind the text
    int background_image_opacity = 15;  // percent, 0..100
    bool tab_bar_visible = true;
    bool sidebar_visible = true;
    int padding = 6;

    // --- Window style ("rice" look) ---
    bool glow_border = false;           // rounded window with a glowing gradient border
    std::string glow_color = "#ff5fd2"; // gradient start (top-left)
    std::string glow_color2 = "#8b7cf6";// gradient end (bottom-right)
    int corner_radius = 14;             // px, 0..32
    bool frameless_window = false;      // no title bar; drag the header (or edges) to move/resize
    bool minimal_chrome = false;        // hide header and sidebar: just the terminal

    // --- Animations ---
    bool animations_enabled = true;
    bool reduced_motion = false;        // also forced on by ZEPHYR_REDUCED_MOTION=1

    // --- Startup ---
    bool restore_session = true;
    std::string default_profile = "Default";

    // --- Performance ---
    int max_fps = 60;                   // repaint coalescing cap: 30 | 60 | 120

    ColorPalette palette = ColorPalette::vivid_neon(); // legacy CLI palette

    // Clamps every numeric field into its valid range and normalises enums.
    void sanitize();
};

// The "Neon Night" look: translucent purple-navy terminal, glowing pink→violet
// rounded border, no header/sidebar. Starts from `base` so fonts, shell and
// other personal choices are kept.
TerminalSettings neon_night_preset(TerminalSettings base);

class TerminalConfig {
public:
    explicit TerminalConfig(std::string path);

    bool load();
    bool save();

    TerminalSettings get() const { return settings_; }
    void set(const TerminalSettings& s) { settings_ = s; }

    const std::string& path() const { return config_.path(); }

private:
    Config config_;
    TerminalSettings settings_;

    void settings_from_config();
    void settings_to_config();
};

} // namespace zephyr::config
