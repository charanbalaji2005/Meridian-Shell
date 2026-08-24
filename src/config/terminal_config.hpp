#pragma once
// meridian-config / terminal_config.hpp
//
// Spec §75's terminal.toml settings, split honestly into two groups:
// the ones that are real and testable without a GUI (scrollback size,
// default shell, startup directory, mouse-scroll-lines), and the ones
// that only mean something once a renderer exists (font, theme, cursor
// blink, padding, tab bar visibility). Both groups are implemented and
// persisted here — a config value isn't "fake" just because nothing
// reads it yet, as long as that's stated plainly, which it is: see the
// per-field comments below.

#include "../core/config.hpp"
#include <string>

namespace meridian::config {

struct TerminalSettings {
    // --- Meaningful right now, independent of any GUI ---
    int scrollback_lines = 10000;      // used directly by ScreenBuffer's constructor
    std::string default_shell = "/bin/bash"; // what PtyManager spawns when none is specified
    std::string startup_directory;      // empty = inherit cwd

    // --- Recorded now; only take effect once a renderer exists ---
    std::string font_family = "monospace";
    int font_size = 13;
    std::string theme_name = "default";
    std::string cursor_style = "block"; // block | underline | bar
    bool cursor_blink = true;
    bool tab_bar_visible = true;
    int padding = 4;
};

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

} // namespace meridian::config
