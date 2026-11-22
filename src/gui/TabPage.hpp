#pragma once
// src/gui/TabPage.hpp
//
// The content of one tab: a tree of QSplitters whose leaves are
// TerminalPanes. Splits nest freely; closing a pane collapses splitters
// that are left with a single child.

#include <QPointer>
#include <QWidget>

#include <functional>

#include "../config/session_state.hpp"
#include "PtyBridge.hpp"

class QSplitter;
class QVBoxLayout;

namespace zephyr::gui {

class TerminalPane;

class TabPage : public QWidget {
    Q_OBJECT
public:
    using SpecResolver = std::function<LaunchSpec(const std::string& profile, const std::string& cwd)>;

    explicit TabPage(const LaunchSpec& spec, QWidget* parent = nullptr);
    TabPage(const config::PaneLayout& layout, const SpecResolver& resolve, QWidget* parent = nullptr);

    TerminalPane* activePane() const;
    QList<TerminalPane*> panes() const;

    // Qt::Horizontal = new pane to the right; Qt::Vertical = new pane below.
    // The new pane runs the same profile in the active pane's directory.
    TerminalPane* split(Qt::Orientation orientation);
    void closePane(TerminalPane* pane);
    void focusNeighbour(int direction); // +1 next, -1 previous
    void focusActive();

    QString title() const;
    bool isSsh() const;
    bool hasForegroundJobs() const;
    config::PaneLayout layout() const;
    void applySettings();

signals:
    void titleChanged();
    void empty();                                        // last pane closed
    void paneContextMenu(TerminalPane* pane, const QPoint& global_pos);

private:
    TerminalPane* makePane(const LaunchSpec& spec);
    QWidget* build(const config::PaneLayout& layout, const SpecResolver& resolve);
    QSplitter* makeSplitter(Qt::Orientation o);
    void replaceInParent(QWidget* old_w, QWidget* new_w);
    void refreshMultiPane();
    config::PaneLayout layoutOf(QWidget* w) const;

    QVBoxLayout* layout_;
    QWidget* root_ = nullptr;
    QPointer<TerminalPane> active_;
};

} // namespace zephyr::gui
