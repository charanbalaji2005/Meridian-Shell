#pragma once
// src/gui/UiTheme.hpp
//
// Colors and the global stylesheet for Zephyr's window chrome (header,
// tabs, sidebar, dialogs, menus). The terminal grid itself is colored by a
// config::Theme; this file only bridges the two.

#include <QColor>
#include <QString>

#include "../config/theme.hpp"

class QMenu;
class QWidget;

namespace zephyr::gui {

struct ChromeColors {
    bool dark = true;
    QColor window;        // outermost background
    QColor surface;       // header, sidebar, dialogs
    QColor surfaceAlt;    // inputs, cards, inactive tabs
    QColor hover;         // hover fill for buttons / rows
    QColor border;        // hairlines
    QColor text;
    QColor muted;         // secondary text, idle icons
    QColor accent;
    QColor accentText;    // text drawn on top of accent
    QColor danger;
    QColor success;
    QColor warning;
};

class UiTheme {
public:
    // Must be called once after QApplication exists: remembers the
    // desktop's own palette so "system" mode can follow it.
    static void captureSystemPalette();

    static bool isDark();
    static ChromeColors colors();
    static QString styleSheet();

    // Re-applies palette + stylesheet + menu effects to the whole app.
    static void apply();

    // The terminal color scheme for the current settings, resolving
    // "zephyr-auto" against the chrome's light/dark mode.
    static const config::Theme& terminalTheme();

    static QColor toQColor(const config::Rgb& c, int alpha = 255) {
        return QColor(c.r, c.g, c.b, alpha);
    }
    static QString css(const QColor& c); // rgba(...) string for stylesheets

    // Rounded, translucent-capable popup menu matching the theme.
    static void polishMenu(QMenu* menu);
};

} // namespace zephyr::gui
