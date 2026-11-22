#pragma once
// src/gui/AppContext.hpp
//
// Process-wide state shared by every Zephyr window: settings, keybindings
// and profiles. Wraps the toolkit-free config classes (src/config/) and
// turns changes into Qt signals so open windows re-apply them live.
// Writes are debounced and always land on disk before the app quits.

#include <QObject>
#include <QTimer>

#include <memory>

#include "../config/keybindings.hpp"
#include "../config/profiles.hpp"
#include "../config/session_state.hpp"
#include "../config/terminal_config.hpp"

namespace zephyr::gui {

class AppContext : public QObject {
    Q_OBJECT

public:
    static AppContext& instance();

    const config::TerminalSettings& settings() const { return settings_; }
    void setSettings(const config::TerminalSettings& s);

    config::Keybindings& keybindings() { return *keybindings_; }
    void notifyKeybindingsChanged();

    const config::ProfileStore& profiles() const { return *profiles_; }
    void setProfiles(std::vector<config::Profile> p, std::vector<config::SshProfile> ssh);

    config::SessionStore& sessionStore() { return *session_; }

    // Default profile, honouring settings().default_profile.
    config::Profile defaultProfile() const;

    // True if animations should run (setting on, reduced motion off, and
    // ZEPHYR_REDUCED_MOTION not set in the environment).
    bool animationsEnabled() const;
    // Scales a nominal animation duration; returns 0 when motion is off.
    int motionMs(int nominal) const { return animationsEnabled() ? nominal : 0; }

    void flush(); // write any pending changes immediately

signals:
    void settingsChanged();
    void keybindingsChanged();
    void profilesChanged();

private:
    AppContext();
    void scheduleSave();

    std::unique_ptr<config::TerminalConfig> terminal_config_;
    std::unique_ptr<config::Keybindings> keybindings_;
    std::unique_ptr<config::ProfileStore> profiles_;
    std::unique_ptr<config::SessionStore> session_;
    config::TerminalSettings settings_;
    QTimer save_timer_;
    bool settings_dirty_ = false;
    bool keybindings_dirty_ = false;
    bool profiles_dirty_ = false;
};

} // namespace zephyr::gui
