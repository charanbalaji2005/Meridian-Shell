#pragma once
// src/gui/SettingsDialog.hpp
//
// Preferences. Every control applies immediately (live preview in all open
// windows) and AppContext persists changes to ~/.config/zephyr/*.toml.

#include <QDialog>

#include <vector>

#include "../config/profiles.hpp"
#include "../config/terminal_config.hpp"

class QListWidget;
class QStackedWidget;
class QLabel;
class QLineEdit;
class QTableWidget;

namespace zephyr::gui {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    void showPage(const QString& id);

signals:
    void connectSsh(const QString& name); // "Connect" on the SSH page
    void reopenRequested(const QString& page); // settings changed wholesale: rebuild the dialog

private:
    void addPage(const QString& id, const QString& icon, const QString& title, QWidget* page);
    void commit(); // push s_ to AppContext

    QWidget* buildAppearance();
    QWidget* buildColors();
    QWidget* buildTerminal();
    QWidget* buildFonts();
    QWidget* buildAnimations();
    QWidget* buildKeybindings();
    QWidget* buildProfiles();
    QWidget* buildSsh();
    QWidget* buildStartup();
    QWidget* buildLinux();
    QWidget* buildNix();
    QWidget* buildPerformance();

    void saveProfiles();

    config::TerminalSettings s_;
    std::vector<config::Profile> profiles_;
    std::vector<config::SshProfile> ssh_;
    QListWidget* nav_;
    QStackedWidget* stack_;
    QStringList page_ids_;
};

} // namespace zephyr::gui
