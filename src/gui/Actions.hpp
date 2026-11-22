#pragma once
// src/gui/Actions.hpp
//
// The catalog of bindable window actions. `id` matches the key used in
// keybindings.toml (config::Keybindings), so the main window, the command
// palette and the keybinding editor all share one definition.

#include <QCoreApplication>
#include <QList>
#include <QString>

namespace zephyr::gui {

struct ActionInfo {
    const char* id;
    const char* label;  // translatable via QCoreApplication::translate("Actions", label)
    const char* icon;   // resources/icons/ui/<icon>.svg, may be empty
    const char* group;  // palette group
};

inline const QList<ActionInfo>& actionCatalog() {
    static const QList<ActionInfo> actions = {
        {"new_tab", QT_TRANSLATE_NOOP("Actions", "New Tab"), "plus", "Tab"},
        {"new_window", QT_TRANSLATE_NOOP("Actions", "New Window"), "app-window", "Window"},
        {"close_tab", QT_TRANSLATE_NOOP("Actions", "Close Tab"), "x", "Tab"},
        {"next_tab", QT_TRANSLATE_NOOP("Actions", "Next Tab"), "chevron-right", "Tab"},
        {"prev_tab", QT_TRANSLATE_NOOP("Actions", "Previous Tab"), "", "Tab"},
        {"split_vertical", QT_TRANSLATE_NOOP("Actions", "Split Side by Side"), "columns-2", "Pane"},
        {"split_horizontal", QT_TRANSLATE_NOOP("Actions", "Split Top and Bottom"), "rows-2", "Pane"},
        {"close_pane", QT_TRANSLATE_NOOP("Actions", "Close Pane"), "x", "Pane"},
        {"focus_next_pane", QT_TRANSLATE_NOOP("Actions", "Focus Next Pane"), "", "Pane"},
        {"focus_prev_pane", QT_TRANSLATE_NOOP("Actions", "Focus Previous Pane"), "", "Pane"},
        {"copy", QT_TRANSLATE_NOOP("Actions", "Copy"), "copy", "Edit"},
        {"paste", QT_TRANSLATE_NOOP("Actions", "Paste"), "clipboard", "Edit"},
        {"select_all", QT_TRANSLATE_NOOP("Actions", "Select All"), "", "Edit"},
        {"search", QT_TRANSLATE_NOOP("Actions", "Find in Output"), "search", "Edit"},
        {"find_next", QT_TRANSLATE_NOOP("Actions", "Find Next"), "chevron-down", "Edit"},
        {"find_prev", QT_TRANSLATE_NOOP("Actions", "Find Previous"), "chevron-up", "Edit"},
        {"clear_scrollback", QT_TRANSLATE_NOOP("Actions", "Clear Scrollback"), "eraser", "Edit"},
        {"increase_font_size", QT_TRANSLATE_NOOP("Actions", "Zoom In"), "", "View"},
        {"decrease_font_size", QT_TRANSLATE_NOOP("Actions", "Zoom Out"), "", "View"},
        {"reset_font_size", QT_TRANSLATE_NOOP("Actions", "Reset Zoom"), "", "View"},
        {"toggle_sidebar", QT_TRANSLATE_NOOP("Actions", "Toggle Sidebar"), "panel-left", "View"},
        {"toggle_fullscreen", QT_TRANSLATE_NOOP("Actions", "Toggle Full Screen"), "maximize", "View"},
        {"command_palette", QT_TRANSLATE_NOOP("Actions", "Command Palette"), "command", "View"},
        {"settings", QT_TRANSLATE_NOOP("Actions", "Settings"), "settings", "App"},
        {"ai_popup", QT_TRANSLATE_NOOP("Actions", "Command Assistant"), "bot", "App"},
    };
    return actions;
}

inline QString actionLabel(const char* id) {
    for (const auto& a : actionCatalog()) {
        if (QString::fromLatin1(a.id) == QLatin1String(id)) return QCoreApplication::translate("Actions", a.label);
    }
    return QString::fromLatin1(id);
}

} // namespace zephyr::gui
