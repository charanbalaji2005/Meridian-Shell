// zephyr-config / keybindings.cpp
#include "keybindings.hpp"

namespace zephyr::config {

Keybindings::Keybindings(std::string path) : config_(std::move(path)) {
    // Combo strings use Qt's QKeySequence portable text format so the GUI
    // can parse them directly. Every default that a shell or TUI app uses
    // itself (Ctrl+W delete-word, Ctrl+T transpose, Ctrl+Space set-mark…)
    // is deliberately avoided: terminal-level actions live on Ctrl+Shift.
    defaults_ = {
        {"ai_popup", "Ctrl+Shift+Space"},
        {"search", "Ctrl+Shift+F"},
        {"find_next", "Ctrl+Shift+G"},       // not F3: htop/mc use it
        {"find_prev", "Ctrl+Alt+Shift+G"},
        {"command_palette", "Ctrl+Shift+P"},
        {"settings", "Ctrl+,"},
        {"new_tab", "Ctrl+Shift+T"},
        {"new_window", "Ctrl+Shift+N"},
        {"close_tab", "Ctrl+Shift+W"},
        {"next_tab", "Ctrl+PgDown"},
        {"prev_tab", "Ctrl+PgUp"},
        {"split_horizontal", "Ctrl+Shift+O"},  // stacked: new pane below
        {"split_vertical", "Ctrl+Shift+E"},    // side by side: new pane to the right
        {"focus_next_pane", "Ctrl+Alt+Right"},
        {"focus_prev_pane", "Ctrl+Alt+Left"},
        {"close_pane", "Ctrl+Shift+X"},
        {"copy", "Ctrl+Shift+C"},
        {"paste", "Ctrl+Shift+V"},
        {"select_all", "Ctrl+Shift+A"},
        {"clear_scrollback", "Ctrl+Shift+K"},
        {"toggle_sidebar", "Ctrl+Shift+B"},
        {"toggle_fullscreen", "F11"},
        {"increase_font_size", "Ctrl+="},
        {"decrease_font_size", "Ctrl+-"},
        {"reset_font_size", "Ctrl+0"},
    };
}

bool Keybindings::load() { return config_.load(); }
bool Keybindings::save() { return config_.save(); }

std::string Keybindings::get(const std::string& action) const {
    std::string default_combo;
    auto it = defaults_.find(action);
    if (it != defaults_.end()) default_combo = it->second;
    return config_.get("keybind." + action, default_combo);
}

void Keybindings::set(const std::string& action, const std::string& combo) {
    config_.set("keybind." + action, combo);
}

void Keybindings::reset(const std::string& action) {
    config_.remove("keybind." + action);
}

std::string Keybindings::default_combo(const std::string& action) const {
    auto it = defaults_.find(action);
    return it != defaults_.end() ? it->second : std::string();
}

std::map<std::string, std::string> Keybindings::all() const {
    std::map<std::string, std::string> result;
    for (const auto& [action, default_combo] : defaults_) {
        result[action] = config_.get("keybind." + action, default_combo);
    }
    return result;
}

} // namespace zephyr::config
