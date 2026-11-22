#include "AppContext.hpp"

#include "../config/paths.hpp"

#include <QCoreApplication>

#include <cstdlib>
#include <cstring>

namespace zephyr::gui {

AppContext& AppContext::instance() {
    static AppContext ctx;
    return ctx;
}

AppContext::AppContext() {
    terminal_config_ = std::make_unique<config::TerminalConfig>(config::config_file("terminal.toml"));
    keybindings_ = std::make_unique<config::Keybindings>(config::config_file("keybindings.toml"));
    profiles_ = std::make_unique<config::ProfileStore>(config::config_file("profiles.toml"));
    session_ = std::make_unique<config::SessionStore>(config::config_file("session.toml"));

    terminal_config_->load();
    keybindings_->load();
    profiles_->load();
    settings_ = terminal_config_->get();

    save_timer_.setSingleShot(true);
    save_timer_.setInterval(400);
    connect(&save_timer_, &QTimer::timeout, this, &AppContext::flush);
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &AppContext::flush);
    }
}

void AppContext::setSettings(const config::TerminalSettings& s) {
    settings_ = s;
    settings_.sanitize();
    settings_dirty_ = true;
    scheduleSave();
    emit settingsChanged();
}

void AppContext::notifyKeybindingsChanged() {
    keybindings_dirty_ = true;
    scheduleSave();
    emit keybindingsChanged();
}

void AppContext::setProfiles(std::vector<config::Profile> p, std::vector<config::SshProfile> ssh) {
    profiles_->set_profiles(std::move(p));
    profiles_->set_ssh_profiles(std::move(ssh));
    profiles_dirty_ = true;
    scheduleSave();
    emit profilesChanged();
}

config::Profile AppContext::defaultProfile() const {
    if (const auto* p = profiles_->find(settings_.default_profile)) return *p;
    return profiles_->profiles().front();
}

bool AppContext::animationsEnabled() const {
    const char* env = std::getenv("ZEPHYR_REDUCED_MOTION");
    bool env_reduced = env && *env && std::strcmp(env, "0") != 0;
    return settings_.animations_enabled && !settings_.reduced_motion && !env_reduced;
}

void AppContext::scheduleSave() { save_timer_.start(); }

void AppContext::flush() {
    save_timer_.stop();
    if (settings_dirty_) {
        terminal_config_->set(settings_);
        terminal_config_->save();
        settings_dirty_ = false;
    }
    if (keybindings_dirty_) {
        keybindings_->save();
        keybindings_dirty_ = false;
    }
    if (profiles_dirty_) {
        profiles_->save();
        profiles_dirty_ = false;
    }
}

} // namespace zephyr::gui
