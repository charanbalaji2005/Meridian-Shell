#pragma once
// src/gui/MainWindow.hpp
//
// A Zephyr window: header (tabs + tool buttons), sidebar, the tab pages,
// the command palette overlay, and every keyboard-bindable action.

#include <QHash>
#include <QMainWindow>
#include <QPointer>

#include "../config/session_state.hpp"
#include "PtyBridge.hpp"

class QAction;
class QMenu;
class QStackedWidget;
class QTabBar;

namespace zephyr::gui {

class CommandPalette;
class IconButton;
class SettingsDialog;
class Sidebar;
class TabPage;
class TerminalPane;
class WindowFrame;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // Opens `initial` if given; else restores `session` if given and
    // non-empty; else opens one tab with the default profile.
    explicit MainWindow(const config::SessionState* session = nullptr, const LaunchSpec* initial = nullptr,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

    void openTab(const LaunchSpec& spec);
    void openProfile(const QString& name);
    void openSsh(const QString& name);
    config::SessionState sessionState() const;

    static int windowCount();

protected:
    void closeEvent(QCloseEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override; // header drag in frameless mode

private:
    void buildUi();
    void createActions();
    void applyKeybindings();
    void applySettings();
    void refreshIcons();
    void rebuildPalette();
    void rebuildNewTabMenu();

    int addPage(TabPage* page);
    void closeTab(int index);
    TabPage* pageAt(int index) const;
    TabPage* currentPage() const;
    TerminalPane* currentPane() const;
    void onCurrentTabChanged(int index);
    void updateTabTitle(TabPage* page);
    void updateWindowTitle();
    void showPaneMenu(TerminalPane* pane, const QPoint& global_pos);
    void openSettings(const QString& page = QString());
    void showAbout();
    bool confirmClose(const QString& what);
    LaunchSpec resolveSpec(const std::string& profile, const std::string& cwd) const;
    LaunchSpec defaultSpec() const;

    QWidget* header_ = nullptr;
    QTabBar* tabs_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    Sidebar* sidebar_ = nullptr;
    CommandPalette* palette_ = nullptr;
    QMenu* new_tab_menu_ = nullptr;
    QList<IconButton*> header_buttons_;
    QHash<QString, QAction*> actions_;
    QList<QAction*> dynamic_actions_; // profiles / hosts / themes for the palette
    QPointer<SettingsDialog> settings_;
    WindowFrame* frame_ = nullptr;
    QWidget* window_controls_ = nullptr; // min / max / close, shown when frameless
    bool frameless_ = false;
    bool closing_ = false;
};

} // namespace zephyr::gui
