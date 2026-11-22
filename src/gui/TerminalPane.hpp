#pragma once
// src/gui/TerminalPane.hpp
//
// One leaf of a tab's split layout: the TerminalView, a scrollbar synced
// to its scrollback viewport, and an inline find bar.

#include <QFrame>
#include <QWidget>

#include "PtyBridge.hpp"

class QLabel;
class QLineEdit;
class QScrollBar;

namespace zephyr::gui {

class IconButton;
class TerminalView;

class SearchBar : public QFrame {
    Q_OBJECT
public:
    explicit SearchBar(QWidget* parent = nullptr);
    void open(const QString& initial);
    void setResult(int current, int total);
    QString query() const;
    bool caseSensitive() const;

signals:
    void queryChanged(const QString& query, bool case_sensitive);
    void next();
    void previous();
    void closed();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    QLineEdit* edit_;
    IconButton* case_btn_;
    QLabel* count_;
};

class TerminalPane : public QWidget {
    Q_OBJECT
public:
    explicit TerminalPane(const LaunchSpec& spec, QWidget* parent = nullptr);

    TerminalView* view() const { return view_; }
    QString title() const;
    void setMultiPane(bool multi);
    void openSearch();
    void closeSearch();
    void applySettings();

signals:
    void focused(TerminalPane* self);
    void finished(TerminalPane* self, int exit_code);
    void titleChanged(TerminalPane* self);
    void contextMenuRequested(TerminalPane* self, const QPoint& global_pos);

protected:
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void syncScrollbar();
    void placeSearchBar();

    TerminalView* view_;
    QScrollBar* scrollbar_;
    SearchBar* search_;
    bool multi_pane_ = false;
    bool syncing_ = false;
};

} // namespace zephyr::gui
