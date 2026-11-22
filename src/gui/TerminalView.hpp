#pragma once
// src/gui/TerminalView.hpp
//
// The terminal grid widget. Owns one PtyBridge (one shell) and renders its
// ScreenBuffer with QPainter. Rendering is driven purely by output: PTY
// data schedules one coalesced repaint (capped at settings().max_fps), and
// the only idle timer is the optional cursor blink.

#include <QElapsedTimer>
#include <QFont>
#include <QPixmap>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <array>

#include "PtyBridge.hpp"

namespace zephyr::gui {

class TerminalView : public QWidget {
    Q_OBJECT

public:
    explicit TerminalView(const LaunchSpec& spec, QWidget* parent = nullptr);
    ~TerminalView() override;

    PtyBridge* bridge() const { return bridge_; }
    bool startFailed() const { return start_failed_; }

    // Re-reads fonts, colors, cursor, padding, scrollback… from AppContext.
    void applySettings();

    // Clipboard & editing
    bool hasSelection() const { return sel_.active; }
    QString selectedText() const;
    void copySelection();
    void paste();
    void pasteText(const QString& text);
    // Types `text` at the prompt without executing it (newlines become
    // spaces). Used by history/package/assistant panels.
    void insertText(const QString& text);
    void selectAll();
    void clearScrollback();

    // Font zoom (per view, relative to the configured size)
    void zoomIn();
    void zoomOut();
    void zoomReset();

    // Search within output (scrollback + screen)
    int setSearch(const QString& query, bool case_sensitive); // returns match count
    void findNext();
    void findPrevious();
    void clearSearch();

    // Scrollback viewport, for the pane's scrollbar
    int historySize() const;
    int viewOffset() const;
    void setViewOffset(int offset);

    QSize cellSize() const { return {qRound(cell_w_), qRound(cell_h_)}; }
    QSize sizeHint() const override { return {800, 500}; }

signals:
    void titleChanged(const QString& title);
    void finished(int exit_code);
    void focused();
    void viewChanged();                                  // scrollbar should resync
    void searchResultsChanged(int current, int total);   // current is 0-based, -1 if none
    void contextMenuRequested(const QPoint& global_pos);

protected:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void inputMethodEvent(QInputMethodEvent* e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery q) const override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    bool focusNextPrevChild(bool) override { return false; } // Tab belongs to the shell

private:
    struct Selection {
        bool active = false;
        int a_line = 0, a_col = 0; // anchor
        int b_line = 0, b_col = 0; // moving end
        bool contains(int line, int col) const;
    };
    struct CellPos { int line; int col; };

    void scheduleRepaint();
    void updateFontMetrics();
    void updateGridSize();
    void resetCursorBlink();
    void snapToBottom();
    void writeInput(const QByteArray& bytes);
    void finishSelection();
    void selectWordAt(const CellPos& p);
    void selectLineAt(int line);
    void scrollToMatch(int index);
    void recomputeSearch();
    QColor resolve(const vt::Color& c, bool is_fg, bool bold) const;
    const QFont& styledFont(bool bold, bool italic, bool underline, bool strike);
    CellPos cellAt(const QPoint& pos, bool clamp = true) const;
    QRectF cursorRect() const;

    PtyBridge* bridge_ = nullptr;
    bool start_failed_ = false;

    // Font & metrics
    QFont base_font_;
    std::array<QFont, 16> style_fonts_;
    std::array<bool, 16> style_ready_{};
    qreal cell_w_ = 8, cell_h_ = 16, baseline_ = 12;
    int zoom_delta_ = 0;
    int padding_ = 6;

    // Colors
    std::array<QColor, 256> palette_;
    QColor bg_, fg_, cursor_color_, selection_color_, match_color_, current_match_color_;
    int window_alpha_ = 255;
    bool paint_background_ = true; // false when the glow WindowFrame paints the body
    QPixmap bg_image_;
    QPixmap bg_scaled_;
    qreal bg_image_opacity_ = 0.15;

    // Cursor
    QTimer blink_timer_;
    bool blink_on_ = true;
    QString cursor_style_ = QStringLiteral("block");
    bool cursor_blink_setting_ = true;

    // Repaint coalescing
    QTimer repaint_timer_;

    // Selection
    Selection sel_;
    bool selecting_ = false;
    QElapsedTimer last_double_click_;
    bool copy_on_select_ = false;

    // Search
    QString query_;
    bool query_case_ = false;
    QVector<vt::SearchMatch> matches_;
    int current_match_ = -1;
    QTimer search_timer_;

    int wheel_accum_ = 0;
    int scroll_multiplier_ = 3;
    bool warn_multiline_paste_ = true;
    bool ligatures_ = true;
};

} // namespace zephyr::gui
