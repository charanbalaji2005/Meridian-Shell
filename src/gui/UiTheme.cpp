#include "UiTheme.hpp"

#include "AppContext.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QMenu>
#include <QPalette>
#include <QStyleHints>

namespace zephyr::gui {

namespace {

bool g_system_dark = true;

QColor mix(const QColor& a, const QColor& b, qreal t) {
    return QColor::fromRgbF(a.redF() * (1 - t) + b.redF() * t, a.greenF() * (1 - t) + b.greenF() * t,
                            a.blueF() * (1 - t) + b.blueF() * t, a.alphaF() * (1 - t) + b.alphaF() * t);
}

} // namespace

void UiTheme::captureSystemPalette() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme != Qt::ColorScheme::Unknown) {
        g_system_dark = scheme == Qt::ColorScheme::Dark;
        return;
    }
#endif
    g_system_dark = QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
}

bool UiTheme::isDark() {
    const auto& mode = AppContext::instance().settings().ui_mode;
    if (mode == "light") return false;
    if (mode == "system") return g_system_dark;
    return true;
}

ChromeColors UiTheme::colors() {
    ChromeColors c;
    c.dark = isDark();
    const auto& s = AppContext::instance().settings();
    auto accent = config::parse_hex_color(s.accent_color);
    c.accent = accent ? toQColor(*accent) : QColor("#8b7cf6");
    c.accentText = c.accent.lightness() > 160 ? QColor("#11131a") : QColor("#ffffff");
    if (c.dark) {
        c.window = QColor("#0b0d12");
        c.surface = QColor("#11141b");
        c.surfaceAlt = QColor("#181c25");
        c.hover = QColor(255, 255, 255, 18);
        c.border = QColor(255, 255, 255, 22);
        c.text = QColor("#e6eaf2");
        c.muted = QColor("#8a92a6");
        c.danger = QColor("#f0717d");
        c.success = QColor("#7fd49a");
        c.warning = QColor("#ebc46f");
    } else {
        c.window = QColor("#eef0f4");
        c.surface = QColor("#f8f9fb");
        c.surfaceAlt = QColor("#ffffff");
        c.hover = QColor(0, 0, 0, 14);
        c.border = QColor(0, 0, 0, 26);
        c.text = QColor("#1c2030");
        c.muted = QColor("#687086");
        c.danger = QColor("#c9364a");
        c.success = QColor("#2e8b57");
        c.warning = QColor("#a86a00");
    }
    return c;
}

const config::Theme& UiTheme::terminalTheme() {
    return config::theme_or_default(AppContext::instance().settings().theme_name, isDark());
}

QString UiTheme::css(const QColor& c) {
    return QStringLiteral("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

QString UiTheme::styleSheet() {
    const auto c = colors();
    const QString accentSoft = css(QColor(c.accent.red(), c.accent.green(), c.accent.blue(), 46));
    const QString accentLine = css(QColor(c.accent.red(), c.accent.green(), c.accent.blue(), 150));
    const QString selected = css(mix(c.surfaceAlt, c.accent, 0.18));

    QString ss = QStringLiteral(R"(
* { outline: none; }
QWidget { color: %TEXT%; font-size: 10pt; }
QMainWindow, QDialog { background: %WINDOW%; }
QMainWindow[zephyrGlow="true"] { background: transparent; }
#WindowFrame, #Central { background: transparent; }
QToolTip { background: %SURFACE_ALT%; color: %TEXT%; border: 1px solid %BORDER%;
           border-radius: 6px; padding: 5px 8px; }

#Header { background: %SURFACE%; border-bottom: 1px solid %BORDER%; }
#Sidebar { background: %SURFACE%; border-right: 1px solid %BORDER%; }
#SidebarRail { background: %SURFACE%; border-right: 1px solid %BORDER%; }
#SidebarTitle { font-weight: 600; font-size: 9pt; color: %MUTED%; letter-spacing: 1px; }
#Muted { color: %MUTED%; }
#Card { background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 10px; }
#Heading { font-size: 13pt; font-weight: 600; }
#Danger { color: %DANGER%; }
#Success { color: %SUCCESS%; }
#Warning { color: %WARNING%; }

QTabBar { background: transparent; }
QTabBar::tab { background: transparent; color: %MUTED%; border: 1px solid transparent;
               border-radius: 8px; padding: 6px 10px; margin: 6px 2px; min-width: 90px; max-width: 220px; }
QTabBar::tab:hover { background: %HOVER%; color: %TEXT%; }
QTabBar::tab:selected { background: %SURFACE_ALT%; color: %TEXT%; border: 1px solid %BORDER%; }
QTabBar::close-button { subcontrol-position: right; }
QTabBar QToolButton { background: %SURFACE%; border: none; }

QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QKeySequenceEdit {
    background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 7px;
    padding: 5px 8px; selection-background-color: %ACCENT%; selection-color: %ACCENT_TEXT%; }
QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus { border: 1px solid %ACCENT_LINE%; }
QComboBox::drop-down { border: none; width: 22px; }
QComboBox QAbstractItemView { background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 8px;
                              padding: 4px; selection-background-color: %ACCENT_SOFT%; selection-color: %TEXT%; }

QPushButton { background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 7px; padding: 6px 14px; }
QPushButton:hover { background: %HOVER%; }
QPushButton:pressed { background: %ACCENT_SOFT%; }
QPushButton:disabled { color: %MUTED%; }
QPushButton#Primary { background: %ACCENT%; color: %ACCENT_TEXT%; border: 1px solid %ACCENT%; font-weight: 600; }
QPushButton#Primary:hover { background: %ACCENT_LINE%; }

QCheckBox { spacing: 8px; }
QCheckBox::indicator { width: 16px; height: 16px; border-radius: 5px; border: 1px solid %BORDER%; background: %SURFACE_ALT%; }
QCheckBox::indicator:checked { background: %ACCENT%; border: 1px solid %ACCENT%; }

QListWidget, QTreeView, QTableWidget, QListView {
    background: transparent; border: none; alternate-background-color: transparent; }
QListWidget::item, QTreeView::item, QListView::item { border-radius: 7px; padding: 6px 8px; margin: 1px 4px; }
QListWidget::item:hover, QTreeView::item:hover, QListView::item:hover { background: %HOVER%; }
QListWidget::item:selected, QTreeView::item:selected, QListView::item:selected { background: %SELECTED%; color: %TEXT%; }
QTableWidget { gridline-color: %BORDER%; }
QTableWidget::item:selected { background: %SELECTED%; color: %TEXT%; }
QHeaderView::section { background: transparent; color: %MUTED%; border: none; border-bottom: 1px solid %BORDER%; padding: 6px; }

QMenu { background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 10px; padding: 6px; }
QMenu::item { padding: 6px 28px 6px 12px; border-radius: 6px; }
QMenu::item:selected { background: %ACCENT_SOFT%; }
QMenu::item:disabled { color: %MUTED%; }
QMenu::separator { height: 1px; background: %BORDER%; margin: 5px 8px; }
QMenu::icon { padding-left: 8px; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: %BORDER%; border-radius: 3px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: %MUTED%; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }
QScrollBar::handle:horizontal { background: %BORDER%; border-radius: 3px; min-width: 28px; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

QSplitter::handle { background: %BORDER%; }
QSplitter::handle:hover { background: %ACCENT_LINE%; }

#SettingsNav { background: %SURFACE%; border-right: 1px solid %BORDER%; padding: 8px 4px; }
#SettingsNav::item { padding: 8px 10px; }
#Palette { background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 14px; }
#Palette QLineEdit { background: transparent; border: none; font-size: 12pt; padding: 10px 6px; }
#SearchBar { background: %SURFACE_ALT%; border: 1px solid %BORDER%; border-radius: 10px; }
#SearchBar QLineEdit { border: none; background: transparent; }
)");
    ss.replace("%WINDOW%", css(c.window));
    ss.replace("%SURFACE_ALT%", css(c.surfaceAlt));
    ss.replace("%SURFACE%", css(c.surface));
    ss.replace("%HOVER%", css(c.hover));
    ss.replace("%BORDER%", css(c.border));
    ss.replace("%TEXT%", css(c.text));
    ss.replace("%MUTED%", css(c.muted));
    ss.replace("%ACCENT_SOFT%", accentSoft);
    ss.replace("%ACCENT_LINE%", accentLine);
    ss.replace("%ACCENT_TEXT%", css(c.accentText));
    ss.replace("%ACCENT%", css(c.accent));
    ss.replace("%SELECTED%", selected);
    ss.replace("%DANGER%", css(c.danger));
    ss.replace("%SUCCESS%", css(c.success));
    ss.replace("%WARNING%", css(c.warning));
    return ss;
}

void UiTheme::apply() {
    const auto c = colors();
    QPalette pal;
    pal.setColor(QPalette::Window, c.window);
    pal.setColor(QPalette::WindowText, c.text);
    pal.setColor(QPalette::Base, c.surfaceAlt);
    pal.setColor(QPalette::AlternateBase, c.surface);
    pal.setColor(QPalette::Text, c.text);
    pal.setColor(QPalette::Button, c.surfaceAlt);
    pal.setColor(QPalette::ButtonText, c.text);
    pal.setColor(QPalette::Highlight, c.accent);
    pal.setColor(QPalette::HighlightedText, c.accentText);
    pal.setColor(QPalette::ToolTipBase, c.surfaceAlt);
    pal.setColor(QPalette::ToolTipText, c.text);
    pal.setColor(QPalette::PlaceholderText, c.muted);
    pal.setColor(QPalette::Link, c.accent);
    pal.setColor(QPalette::Disabled, QPalette::Text, c.muted);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, c.muted);
    QApplication::setPalette(pal);
    qApp->setStyleSheet(styleSheet());

    // Native, cheap dropdown/menu transitions — disabled with reduced motion.
    const bool motion = AppContext::instance().animationsEnabled();
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu, motion);
    QApplication::setEffectEnabled(Qt::UI_FadeMenu, motion);
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo, motion);
    QApplication::setEffectEnabled(Qt::UI_AnimateTooltip, motion);
    QApplication::setEffectEnabled(Qt::UI_FadeTooltip, motion);
}

void UiTheme::polishMenu(QMenu* menu) {
    menu->setAttribute(Qt::WA_TranslucentBackground, true);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
}

} // namespace zephyr::gui
