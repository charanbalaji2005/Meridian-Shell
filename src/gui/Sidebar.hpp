#pragma once
// src/gui/Sidebar.hpp
//
// Left sidebar: a narrow icon rail plus a collapsible panel area.
// Panels never run commands on their own — anything they produce is
// typed at the active prompt via insertText(), and the user decides
// whether to press Enter.

#include <QHash>
#include <QWidget>

class QStackedWidget;
class QLabel;

namespace zephyr::gui {

class IconButton;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);

    bool isExpanded() const { return expanded_; }
    void setExpanded(bool expanded, bool animate = true);
    void showPanel(const QString& id); // profiles|files|history|system|packages|assistant
    QString currentPanel() const { return current_; }

public slots:
    void setCurrentDirectory(const QString& dir);
    void refreshProfiles();
    void applyTheme();

signals:
    void launchProfile(const QString& name);
    void launchSsh(const QString& name);
    void insertText(const QString& text);
    void openSettingsPage(const QString& page);
    void expandedChanged(bool expanded);

private:
    struct Entry { QString id; QString icon; QString title; IconButton* button; QWidget* panel; };
    void addPanel(const QString& id, const QString& icon, const QString& title, QWidget* panel);

    QWidget* rail_;
    QWidget* panel_area_;
    QLabel* panel_title_;
    QStackedWidget* stack_;
    QList<Entry> entries_;
    QString current_;
    bool expanded_ = true;
    static constexpr int kPanelWidth = 290;
};

} // namespace zephyr::gui
