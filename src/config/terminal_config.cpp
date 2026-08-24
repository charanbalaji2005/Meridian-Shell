// meridian-config / terminal_config.cpp
#include "terminal_config.hpp"
#include <cstdlib>

namespace meridian::config {

TerminalConfig::TerminalConfig(std::string path) : config_(std::move(path)) {}

void TerminalConfig::settings_from_config() {
    settings_.scrollback_lines = std::atoi(config_.get("scrollback_lines", "10000").c_str());
    settings_.default_shell = config_.get("default_shell", "/bin/bash");
    settings_.startup_directory = config_.get("startup_directory", "");
    settings_.font_family = config_.get("font_family", "monospace");
    settings_.font_size = std::atoi(config_.get("font_size", "13").c_str());
    settings_.theme_name = config_.get("theme_name", "default");
    settings_.cursor_style = config_.get("cursor_style", "block");
    settings_.cursor_blink = config_.get_bool("cursor_blink", true);
    settings_.tab_bar_visible = config_.get_bool("tab_bar_visible", true);
    settings_.padding = std::atoi(config_.get("padding", "4").c_str());
}

void TerminalConfig::settings_to_config() {
    config_.set("scrollback_lines", std::to_string(settings_.scrollback_lines));
    config_.set("default_shell", settings_.default_shell);
    config_.set("startup_directory", settings_.startup_directory);
    config_.set("font_family", settings_.font_family);
    config_.set("font_size", std::to_string(settings_.font_size));
    config_.set("theme_name", settings_.theme_name);
    config_.set("cursor_style", settings_.cursor_style);
    config_.set_bool("cursor_blink", settings_.cursor_blink);
    config_.set_bool("tab_bar_visible", settings_.tab_bar_visible);
    config_.set("padding", std::to_string(settings_.padding));
}

bool TerminalConfig::load() {
    bool existed = config_.load();
    settings_from_config(); // applies defaults either way
    return existed;
}

bool TerminalConfig::save() {
    settings_to_config();
    return config_.save();
}

} // namespace meridian::config
